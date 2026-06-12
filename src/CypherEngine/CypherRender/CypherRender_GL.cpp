/*======================================================================
   File: r_gl.cpp
   Project: CypherEngine
   Author: ksiric <email@example.com>
   Created: 2026-05-05 22:02:15
   Last Modified by: ksiric
   Last Modified: 2026-06-03 17:49:15
   ---------------------------------------------------------------------
   Description:

   ---------------------------------------------------------------------
   License:
   Company:
   Version: 0.1.0
 ======================================================================
                                                                       */
#include "CypherEngine/CypherRender/CypherRender_GL.h"
#include "CypherEngine/CypherLog/CypherLog.h"
#include "CypherEngine/CypherSystem/CypherSystem_OpenGL.h"
#include "CypherEngine/CypherSystem/CypherSystem_Platform.h"

#include <SDL3/SDL.h>      // SDL owns window creation and GL context lifetime.
#include <cstddef>         // offsetof for vertex attribute layout.

namespace cypher::engine::render
{

/*
================
CypherRenderGL_Init

Creates the SDL OpenGL context and loads GL entry points through GLAD.
================
*/
error_code_t CypherRenderGL_Init( const sys::window_t &window, bool vsync, gl_state_t &gl_state )
{
    SDL_Window *sdl_window{ nullptr};

    if ( window.native_window == nullptr || !window.valid ) {
        LOG_ERROR( log::channel_t::RENDER, "OpenGL init failed: invalid native window." );
        return error_code_t::ERR_INVALID_WINDOW_CFG;
    }

    sdl_window = static_cast<SDL_Window *>( window.native_window );

    // macOS requires a forward-compatible core profile context.
    #if CYPHER_PLATFORM_MACOS
        if (
            !SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, SYS_GL_CONTEXT_MAJOR ) ||
            !SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, SYS_GL_CONTEXT_MINOR ) ||
            !SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK,SDL_GL_CONTEXT_PROFILE_CORE ) ||
            !SDL_GL_SetAttribute( SDL_GL_CONTEXT_FLAGS, SYS_GL_CONTEXT_FLAGS ) ||
            !SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 ) ||
            !SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 24 )  ||
            !SDL_GL_SetAttribute( SDL_GL_STENCIL_SIZE, 8 ) )
        {
            LOG_ERROR( log::channel_t::RENDER, "SDL_GL_SetAttribute failed: %s", SDL_GetError() );

            return error_code_t::ERR_OPENGL_INIT;
        }
    // Windows and Linux can request the newer OpenGL profile.
    #elif CYPHER_PLATFORM_WINDOWS || CYPHER_PLATFORM_LINUX
        if (
            !SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, SYS_GL_CONTEXT_MAJOR ) ||
            !SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, SYS_GL_CONTEXT_MINOR ) ||
            !SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK,SDL_GL_CONTEXT_PROFILE_CORE ) ||
            !SDL_GL_SetAttribute( SDL_GL_CONTEXT_FLAGS, SYS_GL_CONTEXT_FLAGS ) ||
            !SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 ) ||
            !SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 24 )  ||
            !SDL_GL_SetAttribute( SDL_GL_STENCIL_SIZE, 8 ) )
        {
            LOG_ERROR( log::channel_t::RENDER, "SDL_GL_SetAttribute failed: %s", SDL_GetError() );

            return error_code_t::ERR_OPENGL_INIT;
        }
    #else
        #error "Unsupported platform for OpenGL context creation."
    #endif

    SDL_GLContext gl_context{ nullptr };
    gl_context = SDL_GL_CreateContext( sdl_window );

    if ( gl_context == nullptr ) {
        LOG_ERROR( log::channel_t::RENDER, "SDL_GL_CreateContext failed: %s", SDL_GetError() );
        return error_code_t::ERR_OPENGL_INIT;
    }

    if ( !SDL_GL_SetSwapInterval( vsync ? 1 : 0 ) ) {
        LOG_ERROR( log::channel_t::RENDER, "SDL_GL_SetSwapInterval failed: %s", SDL_GetError() );
    }

    if ( !SDL_GL_MakeCurrent( sdl_window, gl_context ) ) {
        SDL_GL_DestroyContext( gl_context );
        LOG_ERROR( log::channel_t::RENDER, "SDL_GL_MakeCurrent failed: %s", SDL_GetError() );
        return error_code_t::ERR_OPENGL_INIT;
    }

    // GLAD must load function pointers after the context is current.
    const auto gl_version = gladLoadGL( reinterpret_cast<GLADloadfunc>( SDL_GL_GetProcAddress ) );

    if ( gl_version == 0 ) {
        SDL_GL_DestroyContext( gl_context );
        LOG_ERROR( log::channel_t::RENDER, "Error loading GLAD library, cannot locate OpenGL functions." );
        return error_code_t::ERR_OPENGL_INIT;
    }

    if ( !GLAD_GL_VERSION_4_1 ) {
        SDL_GL_DestroyContext( gl_context );
        LOG_ERROR( log::channel_t::RENDER, "OpenGL 4.1 core profile is required." );
        return error_code_t::ERR_OPENGL_INIT;
    }

    gl_state.context = gl_context;

    const char *gl_vendor = reinterpret_cast<const char *>( glGetString( GL_VENDOR ) );
    const char *gl_renderer = reinterpret_cast<const char *>( glGetString( GL_RENDERER ) );
    const char *gl_version_string = reinterpret_cast<const char *>( glGetString( GL_VERSION ) );
    const char *glsl_version_string = reinterpret_cast<const char *>( glGetString( GL_SHADING_LANGUAGE_VERSION ) );

    LOG_INFO( log::channel_t::RENDER, "OpenGL initialized: version='%s', glsl='%s', vendor='%s', renderer='%s', vsync=%u.",
                     gl_version_string ? gl_version_string : "<unknown>",
                     glsl_version_string ? glsl_version_string : "<unknown>",
                     gl_vendor ? gl_vendor : "<unknown>",
                     gl_renderer ? gl_renderer : "<unknown>",
                     vsync ? 1u : 0u );

    return error_code_t::OK;
}

/*
================
CypherRenderGL_Shutdown
================
*/
void CypherRenderGL_Shutdown( gl_state_t &gl_state )
{
    if ( gl_state.context == nullptr ) {
        return ;
    }

    SDL_GL_DestroyContext( static_cast<SDL_GLContext>( gl_state.context ) );
    gl_state.context = nullptr;

    LOG_INFO( log::channel_t::RENDER, "OpenGL Context shutdown completed." );

    return ;
}

/*
================
CypherRenderGL_BeginFrame

Sets per-frame GL state and clears the back buffer.
================
*/
error_code_t CypherRenderGL_BeginFrame( const sys::window_t &window )
{
    if ( window.native_window == nullptr || !window.valid ) {
        LOG_ERROR( log::channel_t::RENDER, "GL begin frame failed: invalid native window." );
        return error_code_t::ERR_INVALID_WINDOW_CFG;
    }

    if ( window.width == 0u || window.height == 0u ) {
        LOG_ERROR( log::channel_t::RENDER, "GL begin frame failed: invalid viewport %ux%u.", window.width, window.height );
        return error_code_t::ERR_INVALID_VIEWPORT;
    }

    glViewport( 0, 0, static_cast<GLsizei>( window.width ), static_cast<GLsizei>( window.height ) );

    glEnable( GL_DEPTH_TEST );
    glDepthFunc( GL_LEQUAL );

    glClearColor( 0.04f, 0.045f, 0.05f, 1.0f );
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    return error_code_t::OK;
}

/*
================
CypherRenderGL_EndFrame
================
*/
error_code_t CypherRenderGL_EndFrame( const sys::window_t &window )
{
    SDL_Window *sdl_window{ nullptr };
    if ( !window.valid || window.native_window == nullptr ) {
        LOG_ERROR( log::channel_t::RENDER, "GL end frame failed: invalid native window." );
        return error_code_t::ERR_INVALID_WINDOW_CFG;
    }

    sdl_window = static_cast<SDL_Window *>( window.native_window );

    SDL_GL_SwapWindow( sdl_window );

    return error_code_t::OK;
}

/*
================
CypherRenderGL_CompileShader

Compiles a single OpenGL shader stage and returns its temporary object id.
================
*/
GLuint CypherRenderGL_CompileShader( const GLenum shader_type, const char *shader_source )
{
    if ( shader_source == nullptr || shader_source[0] == '\0' ) {
        LOG_ERROR( log::channel_t::RENDER, "OpenGL shader compile failed: shader source is empty." );
        return 0;
    }

    GLuint shader_id = glCreateShader( shader_type );
    glShaderSource( shader_id, 1, &shader_source, nullptr );
    glCompileShader( shader_id );

    GLint compile_status = GL_FALSE;

    glGetShaderiv( shader_id, GL_COMPILE_STATUS, &compile_status );
    if ( compile_status != GL_TRUE ) {
        char info_log[2024]{};
        glGetShaderInfoLog( shader_id, sizeof( info_log ), nullptr, info_log );

        LOG_ERROR( log::channel_t::RENDER, "CypherRenderGL_CompileShader: OpenGL shader compile failed: %s\n", info_log );
        glDeleteShader( shader_id );
        return 0;
    }

    return shader_id;
}

/*
================
CypherRenderGL_CreateShaderProgram

Compiles vertex/fragment shader stages and links them into one GL program.
================
*/
error_code_t CypherRenderGL_CreateShaderProgram( const char *vertex_source, const char *fragment_source, common::u32 &out_shader_program_id )
{
    out_shader_program_id = 0;

    if ( vertex_source == nullptr || vertex_source[0] == '\0' ) {
        LOG_ERROR( log::channel_t::RENDER, "shader program creation failed: vertex source is empty." );
        return error_code_t::ERR_INVALID_FUNC_PARAMETER;
    }

    if ( fragment_source == nullptr || fragment_source[0] == '\0') {
        LOG_ERROR( log::channel_t::RENDER, "shader program creation failed: fragment source is empty." );
        return error_code_t::ERR_INVALID_FUNC_PARAMETER;
    }

    GLuint vertex_shader_id = CypherRenderGL_CompileShader( GL_VERTEX_SHADER, vertex_source );
    if ( vertex_shader_id == 0 ) {
        return error_code_t::ERR_SHADER_COMPILE;
    }

    GLuint fragment_shader_id = CypherRenderGL_CompileShader( GL_FRAGMENT_SHADER, fragment_source );
    if ( fragment_shader_id == 0 ) {
        glDeleteShader( vertex_shader_id );
        return error_code_t::ERR_SHADER_COMPILE;
    }

    GLuint shader_program_id = glCreateProgram();
    glAttachShader( shader_program_id, vertex_shader_id );
    glAttachShader( shader_program_id, fragment_shader_id );
    glLinkProgram( shader_program_id );

    glDeleteShader( vertex_shader_id );
    glDeleteShader( fragment_shader_id );

    GLint link_status = GL_FALSE;

    glGetProgramiv( shader_program_id, GL_LINK_STATUS, &link_status );
    if ( link_status != GL_TRUE ) {
        char info_log[2024]{};
        glGetProgramInfoLog( shader_program_id, sizeof( info_log ), nullptr, info_log );
        LOG_ERROR( log::channel_t::RENDER, "OpenGL shader program link failed: %s", info_log );
        glDeleteProgram( shader_program_id );
        return error_code_t::ERR_SHADER_LINK;
    }

    out_shader_program_id = static_cast<common::u32>( shader_program_id );

    return error_code_t::OK;
}

/*
================
CypherRenderGL_BindShaderProgram
================
*/
error_code_t CypherRenderGL_BindShaderProgram( const common::u32 shader_program_id )
{
    if ( shader_program_id == 0u ) {
        LOG_ERROR( log::channel_t::RENDER, "shader program bind failed: program id is zero." );
        return error_code_t::ERR_INVALID_FUNC_PARAMETER;
    }

    glUseProgram( shader_program_id );

    return error_code_t::OK;
}

/*
================
CypherRenderGL_DestroyShaderProgram
================
*/
void CypherRenderGL_DestroyShaderProgram( const common::u32 shader_program_id )
{
    if ( shader_program_id == 0u ) {
        return ;
    }

    glDeleteProgram( shader_program_id );

    return ;
}

/*
================
CypherRenderGL_MeshCreate

Uploads mesh vertex/index data and records VAO/VBO/EBO handles.
================
*/
error_code_t CypherRenderGL_MeshCreate( const vertex_t *vertices,
                               const common::u32 vertex_count,
                               const common::u32 *indices,
                               const common::u32 index_count,
                               mesh_t &mesh_out )
{
    if ( vertices == nullptr || vertex_count == 0u ) {
        LOG_ERROR( log::channel_t::RENDER, "GL mesh create failed: invalid vertices pointer/count=%u.", vertex_count );
        return error_code_t::ERR_INVALID_FUNC_PARAMETER;
    }

    if ( indices == nullptr || index_count == 0u ) {
        LOG_ERROR( log::channel_t::RENDER, "GL mesh create failed: invalid indices pointer/count=%u.", index_count );
        return error_code_t::ERR_INVALID_FUNC_PARAMETER;
    }

    GLuint vertex_array_id{ 0u };
    GLuint vertex_buffer_id{ 0u };
    GLuint element_buffer_id{ 0u };

    glGenVertexArrays( 1, &vertex_array_id );
    glGenBuffers( 1, &vertex_buffer_id );
    glGenBuffers( 1, &element_buffer_id );

    if ( vertex_array_id == 0u || vertex_buffer_id == 0u || element_buffer_id == 0u ) {
        if ( element_buffer_id != 0u ) {
            glDeleteBuffers( 1, &element_buffer_id );
        }
        if ( vertex_buffer_id != 0u ) {
            glDeleteBuffers( 1, &vertex_buffer_id );
        }
        if ( vertex_array_id != 0u ) {
            glDeleteVertexArrays( 1, &vertex_array_id );
        }

        LOG_ERROR( log::channel_t::RENDER, "GL mesh create failed: failed generating buffers vao=%u, vbo=%u, ebo=%u.", static_cast<common::u32>( vertex_array_id ), static_cast<common::u32>( vertex_buffer_id ), static_cast<common::u32>( element_buffer_id ) );
        return error_code_t::ERR_OPENGL_INIT;
    }

    glBindVertexArray( vertex_array_id );

    glBindBuffer( GL_ARRAY_BUFFER, vertex_buffer_id );
    glBufferData( GL_ARRAY_BUFFER,
                  static_cast<GLsizeiptr>( vertex_count * sizeof( vertex_t ) ),
                  vertices,
                  GL_STATIC_DRAW );

    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, element_buffer_id );
    glBufferData( GL_ELEMENT_ARRAY_BUFFER,
                  static_cast<GLsizeiptr>( index_count * sizeof( common::u32 ) ),
                  indices,
                  GL_STATIC_DRAW );

    glVertexAttribPointer( 0,
                           3,
                           GL_FLOAT,
                           GL_FALSE,
                           sizeof( vertex_t ),
                           reinterpret_cast<const void *>( offsetof( vertex_t, position ) ) );
    glEnableVertexAttribArray( 0 );

    glVertexAttribPointer( 1,
                           3,
                           GL_FLOAT,
                           GL_FALSE,
                           sizeof( vertex_t ),
                           reinterpret_cast<const void *>( offsetof( vertex_t, color ) ) );
    glEnableVertexAttribArray( 1 );

    glBindVertexArray( 0 );

    mesh_out.vertex_count = vertex_count;
    mesh_out.index_count = index_count;
    mesh_out.gl_vao = static_cast<common::u32>( vertex_array_id );
    mesh_out.gl_vbo = static_cast<common::u32>( vertex_buffer_id );
    mesh_out.gl_ebo = static_cast<common::u32>( element_buffer_id );
    mesh_out.loaded = true;

    LOG_DEBUG( log::channel_t::RENDER, "GL mesh uploaded: vertices=%u, indices=%u, vao=%u, vbo=%u, ebo=%u.", vertex_count, index_count, mesh_out.gl_vao, mesh_out.gl_vbo, mesh_out.gl_ebo );

    return error_code_t::OK;
}

/*
================
CypherRenderGL_MeshDestroy
================
*/
void CypherRenderGL_MeshDestroy( mesh_t &mesh )
{
    if ( mesh.gl_ebo != 0u ) {
        GLuint element_buffer_id = static_cast<GLuint>( mesh.gl_ebo );
        glDeleteBuffers( 1, &element_buffer_id );
    }

    if ( mesh.gl_vbo != 0u ) {
        GLuint vertex_buffer_id = static_cast<GLuint>( mesh.gl_vbo );
        glDeleteBuffers( 1, &vertex_buffer_id );
    }

    if ( mesh.gl_vao != 0u ) {
        GLuint vertex_array_id = static_cast<GLuint>( mesh.gl_vao );
        glDeleteVertexArrays( 1, &vertex_array_id );
    }

    mesh.gl_vao = 0u;
    mesh.gl_vbo = 0u;
    mesh.gl_ebo = 0u;
    mesh.loaded = false;

    return ;
}

/*
================
CypherRenderGL_MeshDraw
================
*/
error_code_t CypherRenderGL_MeshDraw( const mesh_t &mesh )
{
    if ( !mesh.loaded || mesh.gl_vao == 0u || mesh.index_count == 0u ) {
        LOG_ERROR( log::channel_t::RENDER, "GL mesh draw failed: loaded=%u, vao=%u, indices=%u.", mesh.loaded ? 1u : 0u, mesh.gl_vao, mesh.index_count );
        return error_code_t::ERR_INVALID_FUNC_PARAMETER;
    }

    glBindVertexArray( static_cast<GLuint>( mesh.gl_vao ) );
    glDrawElements( GL_TRIANGLES, static_cast<GLsizei>( mesh.index_count ), GL_UNSIGNED_INT, nullptr );
    glBindVertexArray( 0 );

    return error_code_t::OK;
}

error_code_t CypherRenderGL_SetUniformMat4( common::u32 shader_program_id, const char *uniform_name, const math::mat4_t &matrix )
{
    if ( shader_program_id == 0u ) {
        LOG_ERROR( log::channel_t::RENDER, "set uniform mat4 failed: shader program id is zero." );
        return error_code_t::ERR_INVALID_FUNC_PARAMETER;
    }
    
    if ( uniform_name == nullptr || uniform_name[0] == '\0' ) {
        LOG_ERROR( log::channel_t::RENDER, "set uniform mat4 failed: invalid uniform name." );
        return error_code_t::ERR_INVALID_FUNC_PARAMETER;
    }
    
    const GLint uniform_location = glad_glGetUniformLocation( static_cast<GLuint>( shader_program_id ), uniform_name );
    
    if ( uniform_location < 0 ) {
        LOG_ERROR( log::channel_t::RENDER, "set uniform mat4 failed: uniform '%s' not found in program=%u.", uniform_name, shader_program_id );
        return error_code_t::ERR_SHADER_UNIFORM;
    } 
    
    glUseProgram( static_cast<GLuint>( shader_program_id ) );
    
    glad_glUniformMatrix4fv( uniform_location, 1, GL_FALSE, matrix.m );
    
    return error_code_t::OK;
}

}       // namespace cypher::engine::render

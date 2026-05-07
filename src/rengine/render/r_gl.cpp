/*======================================================================
   File: r_gl.cpp
   Project: REAP
   Author: ksiric <email@example.com>
   Created: 2026-05-05 22:02:15
   Last Modified by: ksiric
   Last Modified: 2026-05-08 00:03:28
   ---------------------------------------------------------------------
   Description:
       
   ---------------------------------------------------------------------
   License: 
   Company: 
   Version: 0.1.0
 ======================================================================
                                                                       */
#include "rengine/render/r_gl.h"
#include "rengine/log/log_main.h"
#include "rengine/sys/sys_opengl.h"
#include "rengine/sys/sys_platform.h"

#include <SDL3/SDL.h>

namespace reap::rengine::render
{
    
r_error_code_t R_GLInit( const sys::sys_window_t &window, bool vsync, r_gl_state_t &gl_state ) 
{
    SDL_Window *sdl_window{ nullptr};
    
    if ( window.native_window == nullptr || !window.valid ) {
        return r_error_code_t::ERR_INVALID_WINDOW_CFG;
    }
    
    sdl_window = static_cast<SDL_Window *>( window.native_window );
    
    // @NOTE: For apple we have different specifications 
    #if REAP_PLATFORM_MACOS
        if ( 
            !SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, SYS_GL_CONTEXT_MAJOR ) ||
            !SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, SYS_GL_CONTEXT_MINOR ) ||
            !SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK,SDL_GL_CONTEXT_PROFILE_CORE ) ||
            !SDL_GL_SetAttribute( SDL_GL_CONTEXT_FLAGS, SYS_GL_CONTEXT_FLAGS ) ||
            !SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 ) ||
            !SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 24 )  ||
            !SDL_GL_SetAttribute( SDL_GL_STENCIL_SIZE, 8 ) )  
        {
            REAP_LOG_ERROR( log::log_channel_t::RENDER, "SDL_GL_SetAttribute failed: %s", SDL_GetError() );
            
            return r_error_code_t::ERR_OPENGL_INIT;       
        }
    // @NOTE: For windows and linux we can use different versions and approaches
    #elif REAP_PLATFORM_WINDOWS || REAP_PLATFORM_LINUX 
        if ( 
            !SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, SYS_GL_CONTEXT_MAJOR ) ||
            !SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, SYS_GL_CONTEXT_MINOR ) ||
            !SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK,SDL_GL_CONTEXT_PROFILE_CORE ) ||
            !SDL_GL_SetAttribute( SDL_GL_CONTEXT_FLAGS, SYS_GL_CONTEXT_FLAGS ) ||
            !SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 ) ||
            !SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 24 )  ||
            !SDL_GL_SetAttribute( SDL_GL_STENCIL_SIZE, 8 ) )  
        {
            REAP_LOG_ERROR( log::log_channel_t::RENDER, "SDL_GL_SetAttribute failed: %s", SDL_GetError() );
            
            return r_error_code_t::ERR_OPENGL_INIT;       
        }
    #else 
        #error "Unsupported platform for OpenGL context creation."    
    #endif
    
    SDL_GLContext gl_context{ nullptr };
    gl_context = SDL_GL_CreateContext( sdl_window );
    
    if ( gl_context == nullptr ) {
        REAP_LOG_ERROR( log::log_channel_t::RENDER, "SDL_GL_CreateContext failed: %s", SDL_GetError() );
        return r_error_code_t::ERR_OPENGL_INIT;
    }
    
    if ( !SDL_GL_SetSwapInterval( vsync ? 1 : 0 ) ) {
        REAP_LOG_ERROR( log::log_channel_t::RENDER, "SDL_GL_SetSwapInterval failed: %s", SDL_GetError() );
    }
    
    if ( !SDL_GL_MakeCurrent( sdl_window, gl_context ) ) {
        SDL_GL_DestroyContext( gl_context );
        REAP_LOG_ERROR( log::log_channel_t::RENDER, "SDL_GL_MakeCurrent failed: %s", SDL_GetError() );
        return r_error_code_t::ERR_OPENGL_INIT;
    }
    
    // @NOTE: need to load the GLAD here for the rest of the OpenGL functions will rely on it.
    const auto gl_version = gladLoadGL( reinterpret_cast<GLADloadfunc>( SDL_GL_GetProcAddress ) );
    
    if ( gl_version == 0 ) {
        SDL_GL_DestroyContext( gl_context );
        REAP_LOG_ERROR( log::log_channel_t::RENDER, "Error loading GLAD library, cannot locate OpenGL functions." );
        return r_error_code_t::ERR_OPENGL_INIT;
    }
    
    if ( !GLAD_GL_VERSION_4_1 ) {
        SDL_GL_DestroyContext( gl_context );
        REAP_LOG_ERROR( log::log_channel_t::RENDER, "OpenGL 4.1 core profile is required." );
        return r_error_code_t::ERR_OPENGL_INIT;
    }
    
    gl_state.context = gl_context;   

    return r_error_code_t::OK;
}

void R_GLShutdown( r_gl_state_t &gl_state )
{
    if ( gl_state.context == nullptr ) {
        return ;
    }

    SDL_GL_DestroyContext( static_cast<SDL_GLContext>( gl_state.context ) );
    gl_state.context = nullptr;
    
    REAP_LOG_INFO( log::log_channel_t::RENDER, "OpenGL Context shutdown completed." );
    
    return ;
}

r_error_code_t R_GLBeginFrame( const sys::sys_window_t &window )
{
    if ( window.native_window == nullptr || !window.valid ) {
        return r_error_code_t::ERR_INVALID_WINDOW_CFG;
    }
    
    if ( window.width == 0u || window.height == 0u ) {
        return r_error_code_t::ERR_INVALID_VIEWPORT;
    }
    
    glViewport( 0, 0, static_cast<GLsizei>( window.width ), static_cast<GLsizei>( window.height ) );
    
    glEnable( GL_DEPTH_TEST );
    glDepthFunc( GL_LEQUAL );
    
    glClearColor( 0.04f, 0.045f, 0.05f, 1.0f );
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
 
    return r_error_code_t::OK;
}

r_error_code_t R_GLEndFrame( const sys::sys_window_t &window )
{
    SDL_Window *sdl_window{ nullptr };
    if ( !window.valid || window.native_window == nullptr ) {
        return r_error_code_t::ERR_INVALID_WINDOW_CFG;
    }
    
    sdl_window = static_cast<SDL_Window *>( window.native_window );
    
    SDL_GL_SwapWindow( sdl_window );
    
    return r_error_code_t::OK;
}

r_error_code_t R_GLCreateShaderProgram( const char *vertex_source, const char *fragment_source, rcommon::u32 &out_shader_program_id )
{
    out_shader_program_id = 0;
    
    if ( vertex_source == nullptr || vertex_source[0] == '\0' ) {
        return r_error_code_t::ERR_INVALID_FUNC_PARAMETER;       
    }
    
    if ( fragment_source == nullptr || fragment_source[0] == '\0') {
        return r_error_code_t::ERR_INVALID_FUNC_PARAMETER;       
    }
    
    GLuint vertex_shader_id = R_GLCompileShader( GL_VERTEX_SHADER, vertex_source );
    if ( vertex_shader_id == 0 ) {
        return r_error_code_t::ERR_SHADER_COMPILE;
    }
    
    GLuint fragment_shader_id = R_GLCompileShader( GL_FRAGMENT_SHADER, fragment_source );
    if ( fragment_shader_id == 0 ) {
        return r_error_code_t::ERR_SHADER_COMPILE;
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
        REAP_LOG_ERROR( log::log_channel_t::RENDER, "OpenGL shader program link failed: %s", info_log );
        glDeleteProgram( shader_program_id );
        return r_error_code_t::ERR_SHADER_LINK;
    }
    
    out_shader_program_id = static_cast<rcommon::u32>( shader_program_id );
    
    return r_error_code_t::OK;
}

GLuint R_GLCompileShader( const GLenum shader_type, const char *shader_source )
{
    if ( shader_source == nullptr || shader_source[0] == '\0' ) {
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
        
        REAP_LOG_ERROR( log::log_channel_t::RENDER, "R_GLCompileShader: OpenGL shader compile failed: %s\n", info_log );
        glDeleteShader( shader_id );
        return 0;
    }
    
    return shader_id;
}

r_error_code_t R_GLBindShaderProgram( const rcommon::u32 shader_program_id )
{
    if ( shader_program_id == 0u ) {
        return r_error_code_t::ERR_INVALID_FUNC_PARAMETER;
    }
    
    glUseProgram( shader_program_id );
    
    return r_error_code_t::OK;
}

void R_GLDestroyShaderProgram( const rcommon::u32 shader_program_id )
{
    
    
    return ;   
}

}       // namespace reap::rengine::render


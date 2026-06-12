/*======================================================================
   File: r_shader.cpp
   Project: CypherEngine
   Author: ksiric <email@example.com>
   Created: 2026-05-06 15:03:52
   Last Modified by: ksiric
   Last Modified: 2026-06-03 14:01:51
   ---------------------------------------------------------------------
   Description:

   ---------------------------------------------------------------------
   License:
   Company:
   Version: 0.1.0
 ======================================================================
                                                                       */

#include "CypherEngine/CypherRender/CypherRender_Shader.h"
#include "CypherEngine/CypherFileSystem/CypherFileSystem.h"
#include "CypherEngine/CypherCommon/CypherCommon_Print.h"
#include "CypherEngine/CypherLog/CypherLog.h"
#include "CypherEngine/CypherRender/CypherRender_GL.h"

#include <cstring>     // strcmp / strncpy for fixed-size shader names and paths.

namespace cypher::engine::render
{

namespace
{
char g_vertex_shader_source[CYPHER_RENDER_MAX_SHADER_SOURCE_SIZE + 1u]{};
char g_fragment_shader_source[CYPHER_RENDER_MAX_SHADER_SOURCE_SIZE + 1u]{};
}

/*
================
CypherRender_ShaderRegistryInit
================
*/
void CypherRender_ShaderRegistryInit( shader_registry_t &shader_registry )
{
    shader_registry = {};

    return ;
}

/*
================
CypherRender_ShaderRegistryShutdown

Destroys all loaded GL shader programs before clearing the registry.
================
*/
void CypherRender_ShaderRegistryShutdown( shader_registry_t &shader_registry )
{
    for ( common::u32 i = 0u; i < shader_registry.shader_count; ++i ) {
        CypherRender_ShaderUnload( shader_registry.shaders[i] );
    }

    shader_registry = {};

    return ;
}

/*
================
CypherRender_ShaderLoad

Loads shader source through the file system and creates a GL shader program.
================
*/
error_code_t CypherRender_ShaderLoad( shader_registry_t &shader_registry, const char *name, const char *vertex_path, const char *fragment_path, shader_t **out_shader )
{
    if ( name == nullptr || name[0] == '\0' ) {
        LOG_ERROR( log::channel_t::RENDER, "shader load failed: invalid shader name." );
        COM_ERRORF( CypherRender_ErrorCode( error_code_t::ERR_INVALID_FUNC_PARAMETER ), "CypherRender_ShaderLoad: failed passing invalid shader name.\n" );
        return error_code_t::ERR_INVALID_FUNC_PARAMETER;
    }

    if ( vertex_path == nullptr || vertex_path[0] == '\0' ) {
        LOG_ERROR( log::channel_t::RENDER, "shader load failed: invalid vertex shader path." );
        COM_ERRORF( CypherRender_ErrorCode( error_code_t::ERR_INVALID_FUNC_PARAMETER ), "CypherRender_ShaderLoad: failed passing invalid vertex shader path.\n" );
        return error_code_t::ERR_INVALID_FUNC_PARAMETER;
    }

    if ( fragment_path == nullptr || fragment_path[0] == '\0' ) {
        LOG_ERROR( log::channel_t::RENDER, "shader load failed: invalid fragment shader path." );
        COM_ERRORF( CypherRender_ErrorCode( error_code_t::ERR_INVALID_FUNC_PARAMETER ), "CypherRender_ShaderLoad: failed passing invalid fragment shader path.\n" );
        return error_code_t::ERR_INVALID_FUNC_PARAMETER;
    }

    if ( out_shader == nullptr ) {
        LOG_ERROR( log::channel_t::RENDER, "shader load failed for '%s': out_shader is null.", name );
        COM_ERRORF( CypherRender_ErrorCode( error_code_t::ERR_INVALID_FUNC_PARAMETER ), "CypherRender_ShaderLoad: failed, the out_shader is nullptr.\n" );
        return error_code_t::ERR_INVALID_FUNC_PARAMETER;
    }

    shader_t *existing_shader = CypherRender_ShaderFind( shader_registry, name );
    if ( existing_shader != nullptr ) {
        *out_shader = existing_shader;
        LOG_DEBUG( log::channel_t::RENDER, "shader '%s' already loaded; reusing program=%u.", name, existing_shader->gl_shader_program_id );
        return error_code_t::OK;
    }

    if ( shader_registry.shader_count >= CYPHER_RENDER_MAX_SHADERS ) {
        LOG_ERROR( log::channel_t::RENDER, "shader load failed for '%s': registry full (%u).", name, CYPHER_RENDER_MAX_SHADERS );
        COM_ERRORF( CypherRender_ErrorCode( error_code_t::ERR_SHADER_REGISTRY_FULL), "CypherRender_ShaderLoad: shader_registry is full; [{ %d out of %d }] ", shader_registry.shader_count, CYPHER_RENDER_MAX_SHADERS );
        return error_code_t::ERR_SHADER_REGISTRY_FULL;
    }

    shader_t *shader = &shader_registry.shaders[shader_registry.shader_count];
    *shader = {};

    shader->shader_id = shader_registry.shader_count + 1u;
    std::strncpy( shader->name, name, CYPHER_RENDER_MAX_SHADER_NAME - 1u );
    std::strncpy( shader->vertex_path, vertex_path, CYPHER_RENDER_MAX_SHADER_PATH - 1u );
    std::strncpy( shader->fragment_path, fragment_path, CYPHER_RENDER_MAX_SHADER_PATH - 1u );

    char *vertex_source = g_vertex_shader_source;
    char *fragment_source = g_fragment_shader_source;

    common::u64 vertex_bytes_read{ 0u };
    common::u64 fragment_bytes_read{ 0u };

    const auto vertex_read_result = fs::CypherFileSystem_ReadEntireFile( vertex_path, vertex_source, CYPHER_RENDER_MAX_SHADER_SOURCE_SIZE, vertex_bytes_read );

    if ( vertex_read_result != fs::error_code_t::OK ) {
        LOG_ERROR( log::channel_t::RENDER, "shader '%s' load failed: vertex source '%s' read failed: %s.", name, vertex_path, fs::CypherFileSystem_ErrorDesc( vertex_read_result ) );
        COM_ERRORF( CypherRender_ErrorCode( error_code_t::ERR_SHADER_LOAD ), "CypherRender_ShaderLoad: failed reading vertex shader '%s'.\n", vertex_path );
        return error_code_t::ERR_SHADER_LOAD;
    }
    vertex_source[vertex_bytes_read] = '\0';

    const auto fragment_read_result = fs::CypherFileSystem_ReadEntireFile( fragment_path, fragment_source, CYPHER_RENDER_MAX_SHADER_SOURCE_SIZE, fragment_bytes_read );
    if ( fragment_read_result != fs::error_code_t::OK ) {
        LOG_ERROR( log::channel_t::RENDER, "shader '%s' load failed: fragment source '%s' read failed: %s.", name, fragment_path, fs::CypherFileSystem_ErrorDesc( fragment_read_result ) );
        COM_ERRORF( CypherRender_ErrorCode( error_code_t::ERR_SHADER_LOAD ), "CypherRender_ShaderLoad: failed reading fragment shader '%s'.\n", fragment_path );
        return error_code_t::ERR_SHADER_LOAD;
    }
    fragment_source[fragment_bytes_read] = '\0';

    const auto shader_program_result = CypherRenderGL_CreateShaderProgram( vertex_source, fragment_source, shader->gl_shader_program_id );

    if ( shader_program_result != error_code_t::OK ) {
        LOG_ERROR( log::channel_t::RENDER, "shader '%s' load failed: GL program creation failed: %s.", name, CypherRender_ErrorDesc( shader_program_result ) );
        COM_ERRORF( CypherRender_ErrorCode( shader_program_result), "CypherRender_ShaderLoad: CypherRenderGL_CreateShaderProgram: failed creating shader program '%s'.\n", name );
        *shader = {};
        return shader_program_result;
    }

    shader->loaded = true;
    shader_registry.shader_count++;
    *out_shader = shader;

    LOG_INFO( log::channel_t::RENDER, "shader '%s' loaded: program=%u, vertex='%s' (%llu bytes), fragment='%s' (%llu bytes).",
                     name,
                     shader->gl_shader_program_id,
                     vertex_path,
                     static_cast<unsigned long long>( vertex_bytes_read ),
                     fragment_path,
                     static_cast<unsigned long long>( fragment_bytes_read ) );

    return error_code_t::OK;
}

/*
================
CypherRender_ShaderFind
================
*/
shader_t *CypherRender_ShaderFind( shader_registry_t &registry, const char *name )
{
    if ( name == nullptr || name[0] == '\0' ) {
        return nullptr;
    }

    for ( int i = 0; i < registry.shader_count; ++i ) {
        if ( std::strcmp( registry.shaders[i].name, name ) == 0 ) {
            return &registry.shaders[i];
        }
    }

    return nullptr;
}

/*
================
CypherRender_ShaderBind
================
*/
error_code_t CypherRender_ShaderBind( const shader_t &shader )
{
    if ( shader.gl_shader_program_id == 0  || !shader.loaded ) {
        LOG_ERROR( log::channel_t::RENDER, "shader bind failed: invalid shader program id=%u loaded=%u.", shader.gl_shader_program_id, shader.loaded ? 1u : 0u );
        COM_ERRORF( CypherRender_ErrorCode( error_code_t::ERR_INVALID_FUNC_PARAMETER ), "CypherRender_BindShader: Invalid shader program id passed; %d\n", shader.gl_shader_program_id );
        return error_code_t::ERR_SHADER_LOAD;
    }

    return CypherRenderGL_BindShaderProgram( shader.gl_shader_program_id );
}

/*
================
CypherRender_ShaderUnload
================
*/
void CypherRender_ShaderUnload( shader_t &shader )
{
    if ( !shader.loaded ) {
        return ;
    }

    if ( shader.gl_shader_program_id != 0u ) {
        CypherRenderGL_DestroyShaderProgram( shader.gl_shader_program_id );
    }

    shader = {};
    return ;
}

error_code_t CypherRender_ShaderSetMat4( const shader_t &shader, const char *uniform_name, const math::mat4_t &matrix )
{
    if ( !shader.loaded || shader.gl_shader_program_id == 0u ) {
        return error_code_t::ERR_SHADER_BIND;
    }
    
    if ( uniform_name == nullptr || uniform_name[0] == '\0' ) {
        return error_code_t::ERR_INVALID_FUNC_PARAMETER;
    }
    
    return CypherRenderGL_SetUniformMat4( shader.gl_shader_program_id, uniform_name, matrix );
}

}       // namespace cypher::engine::render

/*======================================================================
   File: r_shader.cpp
   Project: REAP
   Author: ksiric <email@example.com>
   Created: 2026-05-06 15:03:52
   Last Modified by: ksiric
   Last Modified: 2026-05-08 00:16:04
   ---------------------------------------------------------------------
   Description:
       
   ---------------------------------------------------------------------
   License: 
   Company: 
   Version: 0.1.0
 ======================================================================
                                                                       */

#include "rengine/render/r_shader.h"
#include "rengine/fs/fs_main.h"
#include "rengine/rcommon/com_print.h"
#include "rengine/render/r_gl.h"

#include <cstring>

namespace reap::rengine::render
{

void R_ShaderRegistryInit( r_shader_registry_t &shader_registry )
{
    shader_registry = {};
    
    return ;
}     

void R_ShaderRegistryShutdown( r_shader_registry_t &shader_registry )
{
    for ( rcommon::u32 i = 0u; i < shader_registry.shader_count; ++i ) {
        R_ShaderUnload( shader_registry.shaders[i] );
    }
    
    shader_registry = {};
    
    return ;
}

r_error_code_t R_ShaderLoad( r_shader_registry_t &shader_registry, const char *name, const char *vertex_path, const char *fragment_path, r_shader_t **out_shader )
{
    if ( name == nullptr || name[0] == '\0' ) {
        rcommon::Com_Errorf( R_ErrorCode( r_error_code_t::ERR_INVALID_FUNC_PARAMETER ), "R_ShaderLoad: failed passing invalid shader name.\n" );
        return r_error_code_t::ERR_INVALID_FUNC_PARAMETER;
    }
    
    if ( vertex_path == nullptr || vertex_path[0] == '\0' ) {
        rcommon::Com_Errorf( R_ErrorCode( r_error_code_t::ERR_INVALID_FUNC_PARAMETER ), "R_ShaderLoad: failed passing invalid vertex shader path.\n" );
        return r_error_code_t::ERR_INVALID_FUNC_PARAMETER;
    }
    
    if ( fragment_path == nullptr || fragment_path[0] == '\0' ) {
        rcommon::Com_Errorf( R_ErrorCode( r_error_code_t::ERR_INVALID_FUNC_PARAMETER ), "R_ShaderLoad: failed passing invalid fragment shader path.\n" );
        return r_error_code_t::ERR_INVALID_FUNC_PARAMETER;
    }
    
    if ( out_shader == nullptr ) {
        rcommon::Com_Errorf( R_ErrorCode( r_error_code_t::ERR_INVALID_FUNC_PARAMETER ), "R_ShaderLoad: failed, the out_shader is nullptr.\n" );
        return r_error_code_t::ERR_INVALID_FUNC_PARAMETER;
    }   
    
    // @NOTE: Adding later the shader find function used for locating the shaders for the engine runtime.
    r_shader_t *existing_shader = R_ShaderFind( shader_registry, name );
    if ( existing_shader != nullptr ) {
        *out_shader = existing_shader;
        return r_error_code_t::OK;
    } 
    
    if ( shader_registry.shader_count >= R_MAX_SHADERS ) {
        rcommon::Com_Errorf( R_ErrorCode( r_error_code_t::ERR_SHADER_REGISTRY_FULL), "R_ShaderLoad: shader_registry is full; [{ %d out of %d }] ", shader_registry.shader_count, R_MAX_SHADERS );
        return r_error_code_t::ERR_SHADER_REGISTRY_FULL;
    }
    
    r_shader_t *shader = &shader_registry.shaders[shader_registry.shader_count];
    *shader = {};
    
    shader->shader_id = shader_registry.shader_count + 1u;
    std::strncpy( shader->name, name, R_MAX_SHADER_NAME - 1u );
    std::strncpy( shader->vertex_path, vertex_path, R_MAX_SHADER_PATH - 1u );
    std::strncpy( shader->fragment_path, fragment_path, R_MAX_SHADER_PATH - 1u );
    
    /*
        // @NOTE:
        
        Next step is to load the shaders paths via the FS itself.
        
        This is how the engine will be doing this as well everything will happen through the FS itself.
        read vertex_shader
        read fragment_shader
        pass both sources to the so called R_GLCreateShaderProgram
        store that result into the shader->gl_program_id
        
     */
    
    // @NOTE: Step one is hook the FS of the engine to actually read the vertex_shader and frament_shader
    
    char vertex_source[R_MAX_SHADER_SOURCE_SIZE + 1u]{};
    char fragment_source[R_MAX_SHADER_SOURCE_SIZE + 1u]{};
    
    rcommon::u64 vertex_bytes_read{ 0u };
    rcommon::u64 fragment_bytes_read{ 0u };
    
    const auto vertex_read_result = fs::FS_ReadEntireFile( vertex_path, vertex_source, R_MAX_SHADER_SOURCE_SIZE, vertex_bytes_read );
    
    if ( vertex_read_result != fs::fs_error_code_t::OK ) {
        rcommon::Com_Errorf( R_ErrorCode( r_error_code_t::ERR_SHADER_LOAD ), "R_ShaderLoad: failed reading vertex shader '%s'.\n", vertex_path );
        return r_error_code_t::ERR_SHADER_LOAD;
    }
    vertex_source[vertex_bytes_read] = '\0';
    
    const auto fragment_read_result = fs::FS_ReadEntireFile( fragment_path, fragment_source, R_MAX_SHADER_SOURCE_SIZE, fragment_bytes_read );
    if ( fragment_read_result != fs::fs_error_code_t::OK ) {
        rcommon::Com_Errorf( R_ErrorCode( r_error_code_t::ERR_SHADER_LOAD ), "R_ShaderLoad: failed reading fragment shader '%s'.\n", fragment_path );
        return r_error_code_t::ERR_SHADER_LOAD;
    }
    fragment_source[fragment_bytes_read] = '\0';
    
    // @NOTE: Now we need to invoke the creation of the shader programs. So we combine the so called both shaders and create a single shader program out of those shaders.
    
    const auto shader_program_result = R_GLCreateShaderProgram( vertex_source, fragment_source, shader->gl_shader_program_id );
    
    if ( shader_program_result != r_error_code_t::OK ) {
        rcommon::Com_Errorf( R_ErrorCode( shader_program_result), "R_ShaderLoad: R_GLCreateShaderProgram: failed creating shader program '%s'.\n", name );
        *shader = {};
        return shader_program_result;
    }
    
    shader->loaded = true;
    shader_registry.shader_count++;
    *out_shader = shader;
    
    return r_error_code_t::OK;
}

r_shader_t *R_ShaderFind( r_shader_registry_t &registry, const char *name )
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

r_error_code_t R_ShaderBind( r_shader_t &shader )
{
    if ( shader.gl_shader_program_id == 0  || !shader.loaded ) {
        rcommon::Com_Errorf( R_ErrorCode( r_error_code_t::ERR_INVALID_FUNC_PARAMETER ), "R_BindShader: Invalid shader program id passed; %d\n", shader.gl_shader_program_id );
        return r_error_code_t::ERR_SHADER_LOAD;
    }
    
    return R_GLBindShaderProgram( shader.gl_shader_program_id );
}

void R_ShaderUnload( r_shader_t &shader )
{
    if ( !shader.loaded ) {
        return ;
    }   
    
    if ( shader.gl_shader_program_id != 0u ) {
        R_GLDestroyShaderProgram( shader.gl_shader_program_id );
    }
    
    shader = {};
    return ;
}

}       // namespace reap::rengine::render

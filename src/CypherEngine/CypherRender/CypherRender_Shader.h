#pragma once

#include "CypherEngine/CypherMath/CypherMath_Types.h"
#include "CypherEngine/CypherCommon/CypherCommon.h"
#include "CypherEngine/CypherRender/CypherRender_Error.h"

namespace cypher::engine::render
{

/*
================
Shader Limits
================
*/
constexpr common::u32 CYPHER_RENDER_MAX_SHADER_NAME            = 64u;
constexpr common::u32 CYPHER_RENDER_MAX_SHADER_PATH            = 256u;
constexpr common::u32 CYPHER_RENDER_MAX_SHADERS                = 1024u;
constexpr common::u64 CYPHER_RENDER_MAX_SHADER_SOURCE_SIZE     = 1024u * 1024u;    // 1MB for shaders.

/*
================
Shader Types
================
*/
struct cypher_render_shader_t {
    common::com_u32 shader_id{ 0 };
    char name[CYPHER_RENDER_MAX_SHADER_NAME]{};
    char vertex_path[CYPHER_RENDER_MAX_SHADER_PATH]{};
    char fragment_path[CYPHER_RENDER_MAX_SHADER_PATH]{};

    common::u32 gl_shader_program_id{ 0 };
    bool loaded{ false };
};

struct cypher_render_shader_registry_t {
    cypher_render_shader_t shaders[CYPHER_RENDER_MAX_SHADERS]{};
    common::u32 shader_count{ 0 };
};

/*
================
Shader Registry API
================
*/
void CypherRender_ShaderRegistryInit( cypher_render_shader_registry_t &shader_registry );

void CypherRender_ShaderRegistryShutdown( cypher_render_shader_registry_t &shader_registry );

cypher_render_error_code_t CypherRender_ShaderLoad( cypher_render_shader_registry_t &shader_registry, const char *name, const char *vertex_path, const char *fragment_path, cypher_render_shader_t **out_shader );

cypher_render_shader_t *CypherRender_ShaderFind( cypher_render_shader_registry_t &registry, const char *name );

cypher_render_error_code_t CypherRender_ShaderBind( const cypher_render_shader_t &shader );

void CypherRender_ShaderUnload( cypher_render_shader_t &shader );

cypher_render_error_code_t CypherRender_ShaderSetMat4( const cypher_render_shader_t &shader, const char *uniform_name, const math::mat4_t &matrix );

}       // namespace cypher::engine::render

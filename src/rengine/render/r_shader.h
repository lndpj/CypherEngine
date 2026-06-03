#pragma once

#include "rengine/math/math_types.h"
#include "rengine/rcommon/com_main.h"
#include "rengine/render/r_error.h"

namespace reap::rengine::render
{

/*
================
Shader Limits
================
*/
constexpr rcommon::u32 R_MAX_SHADER_NAME            = 64u;
constexpr rcommon::u32 R_MAX_SHADER_PATH            = 256u;
constexpr rcommon::u32 R_MAX_SHADERS                = 1024u;
constexpr rcommon::u64 R_MAX_SHADER_SOURCE_SIZE     = 1024u * 1024u;    // 1MB for shaders.

/*
================
Shader Types
================
*/
struct r_shader_t {
    rcommon::com_u32 shader_id{ 0 };
    char name[R_MAX_SHADER_NAME]{};
    char vertex_path[R_MAX_SHADER_PATH]{};
    char fragment_path[R_MAX_SHADER_PATH]{};

    rcommon::u32 gl_shader_program_id{ 0 };
    bool loaded{ false };
};

struct r_shader_registry_t {
    r_shader_t shaders[R_MAX_SHADERS]{};
    rcommon::u32 shader_count{ 0 };
};

/*
================
Shader Registry API
================
*/
void R_ShaderRegistryInit( r_shader_registry_t &shader_registry );

void R_ShaderRegistryShutdown( r_shader_registry_t &shader_registry );

r_error_code_t R_ShaderLoad( r_shader_registry_t &shader_registry, const char *name, const char *vertex_path, const char *fragment_path, r_shader_t **out_shader );

r_shader_t *R_ShaderFind( r_shader_registry_t &registry, const char *name );

r_error_code_t R_ShaderBind( const r_shader_t &shader );

void R_ShaderUnload( r_shader_t &shader );

r_error_code_t R_ShaderSetMat4( const r_shader_t &shader, const char *uniform_name, const math::mat4_t &matrix );









}       // namespace reap::rengine::render

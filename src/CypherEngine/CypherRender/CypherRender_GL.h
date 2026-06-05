#pragma once

#include "CypherEngine/CypherRender/CypherRender_Error.h"
#include "CypherEngine/CypherRender/CypherRender_Mesh.h"
#include "CypherEngine/CypherSystem/CypherSystem_Window.h"
#include "CypherEngine/CypherMath/CypherMath_Types.h"

namespace cypher::engine::render
{

/*
================
OpenGL Backend State

Opaque backend context kept out of higher-level renderer code.
================
*/
struct cypher_render_gl_state_t {
    void *context{ nullptr };
};

/*
================
OpenGL Context
================
*/
cypher_render_error_code_t CypherRenderGL_Init( const sys::cypher_system_window_t &window, bool vsync, cypher_render_gl_state_t &gl_state );

void CypherRenderGL_Shutdown( cypher_render_gl_state_t &gl_state );

cypher_render_error_code_t CypherRenderGL_BeginFrame( const sys::cypher_system_window_t &window );

cypher_render_error_code_t CypherRenderGL_EndFrame( const sys::cypher_system_window_t &window );

/*
================
OpenGL Shaders
================
*/
cypher_render_error_code_t CypherRenderGL_CreateShaderProgram( const char *vertex_source, const char *fragment_source, common::u32 &out_shader_program_id );

cypher_render_error_code_t CypherRenderGL_BindShaderProgram( const common::u32 shader_program_id );

void CypherRenderGL_DestroyShaderProgram( const common::u32 shader_program_id );

/*
================
OpenGL Meshes
================
*/
cypher_render_error_code_t CypherRenderGL_MeshCreate( const cypher_render_vertex_t *vertices,
                               const common::u32 vertex_count,
                               const common::u32 *indices,
                               const common::u32 index_count,
                               cypher_render_mesh_t &mesh_out );

void CypherRenderGL_MeshDestroy( cypher_render_mesh_t &mesh );

cypher_render_error_code_t CypherRenderGL_MeshDraw( const cypher_render_mesh_t &mesh );

cypher_render_error_code_t CypherRenderGL_SetUniformMat4( common::u32 shader_program_id, const char *uniform_name, const math::mat4_t &matrix );

}       // namespace cypher::engine::render

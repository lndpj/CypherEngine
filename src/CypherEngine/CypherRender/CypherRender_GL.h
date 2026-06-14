#ifndef CYPHER_ENGINE_RENDER_GL_H
#define CYPHER_ENGINE_RENDER_GL_H

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
struct gl_state_t {
    void *context{ nullptr };
};

/*
================
OpenGL Context
================
*/
render_error_t CypherRenderGL_Init( const sys::window_t &window, bool vsync, gl_state_t &gl_state );

void CypherRenderGL_Shutdown( gl_state_t &gl_state );

render_error_t CypherRenderGL_BeginFrame( const sys::window_t &window );

render_error_t CypherRenderGL_EndFrame( const sys::window_t &window );

/*
================
OpenGL Shaders
================
*/
render_error_t CypherRenderGL_CreateShaderProgram( const char *vertex_source, const char *fragment_source, common::u32 &out_shader_program_id );

render_error_t CypherRenderGL_BindShaderProgram( const common::u32 shader_program_id );

void CypherRenderGL_DestroyShaderProgram( const common::u32 shader_program_id );

/*
================
OpenGL Meshes
================
*/
render_error_t CypherRenderGL_MeshCreate( const vertex_t *vertices,
                               const common::u32 vertex_count,
                               const common::u32 *indices,
                               const common::u32 index_count,
                               mesh_t &mesh_out );

void CypherRenderGL_MeshDestroy( mesh_t &mesh );

render_error_t CypherRenderGL_MeshDraw( const mesh_t &mesh );

render_error_t CypherRenderGL_SetUniformMat4( common::u32 shader_program_id, const char *uniform_name, const math::mat4_t &matrix );

}       // namespace cypher::engine::render

#endif // CYPHER_ENGINE_RENDER_GL_H

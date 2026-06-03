#pragma once

#include "rengine/render/r_error.h"
#include "rengine/render/r_mesh.h"
#include "rengine/sys/sys_window.h"
#include "rengine/math/math_types.h"

namespace reap::rengine::render
{

/*
================
OpenGL Backend State

Opaque backend context kept out of higher-level renderer code.
================
*/
struct r_gl_state_t {
    void *context{ nullptr };
};

/*
================
OpenGL Context
================
*/
r_error_code_t R_GLInit( const sys::sys_window_t &window, bool vsync, r_gl_state_t &gl_state );

void R_GLShutdown( r_gl_state_t &gl_state );

r_error_code_t R_GLBeginFrame( const sys::sys_window_t &window );

r_error_code_t R_GLEndFrame( const sys::sys_window_t &window );

/*
================
OpenGL Shaders
================
*/
r_error_code_t R_GLCreateShaderProgram( const char *vertex_source, const char *fragment_source, rcommon::u32 &out_shader_program_id );

r_error_code_t R_GLBindShaderProgram( const rcommon::u32 shader_program_id );

void R_GLDestroyShaderProgram( const rcommon::u32 shader_program_id );

/*
================
OpenGL Meshes
================
*/
r_error_code_t R_GLMeshCreate( const r_vertex_t *vertices,
                               const rcommon::u32 vertex_count,
                               const rcommon::u32 *indices,
                               const rcommon::u32 index_count,
                               r_mesh_t &mesh_out );

void R_GLMeshDestroy( r_mesh_t &mesh );

r_error_code_t R_GLMeshDraw( const r_mesh_t &mesh );

r_error_code_t R_GLSetUniformMat4( rcommon::u32 shader_program_id, const char *uniform_name, const math::mat4_t &matrix );

}       // namespace reap::rengine::render

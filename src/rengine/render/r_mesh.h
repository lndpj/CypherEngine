#pragma once

#include "rengine/math/math_types.h"
#include "rengine/render/r_error.h"

namespace reap::rengine::render
{

/*
================
Renderer Mesh Types

CPU-facing mesh description plus backend handles owned by the renderer.
================
*/
struct r_vertex_t {
    math::vec3_t position{};
    math::vec3_t color{};
};

struct r_mesh_t {
    rcommon::u32 vertex_count{ 0u };
    rcommon::u32 index_count{ 0u };

    rcommon::u32 gl_vao{ 0u };
    rcommon::u32 gl_vbo{ 0u };
    rcommon::u32 gl_ebo{ 0u };

    math::bounds_t bounds{};

    bool loaded{ false };
};

/*
================
R_MeshCreate

Uploads vertex/index data into the active renderer backend.
================
*/
r_error_code_t R_MeshCreate( const r_vertex_t *vertices,
                             const rcommon::u32 vertex_count,
                             const rcommon::u32 *indices,
                             const rcommon::u32 index_count,
                             r_mesh_t &mesh_out );

void R_MeshDestroy( r_mesh_t &mesh );

r_error_code_t R_MeshDraw( const r_mesh_t &mesh );

}       // namespace reap::rengine::render

#pragma once

#include "CypherEngine/CypherMath/CypherMath_Types.h"
#include "CypherEngine/CypherRender/CypherRender_Error.h"

namespace cypher::engine::render
{

/*
================
Renderer Mesh Types

CPU-facing mesh description plus backend handles owned by the renderer.
================
*/
struct vertex_t {
    math::vec3_t position{};
    math::vec3_t color{};
};

struct mesh_t {
    common::u32 vertex_count{ 0u };
    common::u32 index_count{ 0u };

    common::u32 gl_vao{ 0u };
    common::u32 gl_vbo{ 0u };
    common::u32 gl_ebo{ 0u };

    math::bounds_t bounds{};

    bool loaded{ false };
};

/*
================
CypherRender_MeshCreate

Uploads vertex/index data into the active renderer backend.
================
*/
error_code_t CypherRender_MeshCreate( const vertex_t *vertices,
                             const common::u32 vertex_count,
                             const common::u32 *indices,
                             const common::u32 index_count,
                             mesh_t &mesh_out );

void CypherRender_MeshDestroy( mesh_t &mesh );

error_code_t CypherRender_MeshDraw( const mesh_t &mesh );

}       // namespace cypher::engine::render

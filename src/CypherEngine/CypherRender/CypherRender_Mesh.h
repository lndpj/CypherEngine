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
struct cypher_render_vertex_t {
    math::vec3_t position{};
    math::vec3_t color{};
};

struct cypher_render_mesh_t {
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
cypher_render_error_code_t CypherRender_MeshCreate( const cypher_render_vertex_t *vertices,
                             const common::u32 vertex_count,
                             const common::u32 *indices,
                             const common::u32 index_count,
                             cypher_render_mesh_t &mesh_out );

void CypherRender_MeshDestroy( cypher_render_mesh_t &mesh );

cypher_render_error_code_t CypherRender_MeshDraw( const cypher_render_mesh_t &mesh );

}       // namespace cypher::engine::render

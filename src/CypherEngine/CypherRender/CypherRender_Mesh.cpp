/*======================================================================
   File: r_mesh.cpp
   Project: CypherEngine
   Author: ksiric <email@example.com>
   Created: 2026-05-10 21:22:24
   Last Modified by: ksiric
   Last Modified: 2026-06-04 19:39:39
   ---------------------------------------------------------------------
   Description:

   ---------------------------------------------------------------------
   License:
   Company:
   Version: 0.1.0
 ======================================================================
                                                                       */

#include "CypherEngine/CypherRender/CypherRender_Mesh.h"
#include "CypherEngine/CypherRender/CypherRender_GL.h"

namespace cypher::engine::render
{

/*
================
CypherRender_MeshCreate

Validates CPU mesh data, calculates simple bounds, then uploads to OpenGL.
================
*/
error_code_t CypherRender_MeshCreate( const vertex_t *vertices,
                             const common::u32 vertex_count,
                             const common::u32 *indices,
                             const common::u32 index_count,
                             mesh_t &mesh_out )
{
    if ( vertices == nullptr || vertex_count == 0u ) {
        return error_code_t::ERR_INVALID_FUNC_PARAMETER;
    }

    if ( indices == nullptr || index_count == 0u ) {
        return error_code_t::ERR_INVALID_FUNC_PARAMETER;
    }

    mesh_out = {};

    mesh_out.bounds.mins = vertices[0].position;
    mesh_out.bounds.maxs = vertices[0].position;

    for ( common::u32 i = 1u; i < vertex_count; ++i ) {
        const math::vec3_t &position = vertices[i].position;

        if ( position.x < mesh_out.bounds.mins.x ) {
            mesh_out.bounds.mins.x = position.x;
        }
        if ( position.y < mesh_out.bounds.mins.y ) {
            mesh_out.bounds.mins.y = position.y;
        }
        if ( position.z < mesh_out.bounds.mins.z ) {
            mesh_out.bounds.mins.z = position.z;
        }
        if ( position.x > mesh_out.bounds.maxs.x ) {
            mesh_out.bounds.maxs.x = position.x;
        }
        if ( position.y > mesh_out.bounds.maxs.y ) {
            mesh_out.bounds.maxs.y = position.y;
        }
        if ( position.z > mesh_out.bounds.maxs.z ) {
            mesh_out.bounds.maxs.z = position.z;
        }
    }
    
    // Calling OpenGL API for creating a mesh, creating a distinction between different API's.
    const auto result = CypherRenderGL_MeshCreate(
        vertices,
        vertex_count,
        indices,
        index_count,
        mesh_out );

    if ( result != error_code_t::OK ) {
        mesh_out = {};
    }

    return result;
}

void CypherRender_MeshDestroy( mesh_t &mesh )
{
    if ( !mesh.loaded ) {
        return ;
    }

    CypherRenderGL_MeshDestroy( mesh );
    mesh = {};

    return ;
}

error_code_t CypherRender_MeshDraw( const mesh_t &mesh )
{
    if ( !mesh.loaded ) {
        return error_code_t::ERR_INVALID_FUNC_PARAMETER;
    }

    return CypherRenderGL_MeshDraw( mesh );
}

}       // namespace cypher::engine::render

/*======================================================================
   File: r_draw.cpp
   Project: CypherEngine
   Author: ksiric <email@example.com>
   Created: 2026-06-04 20:27:26
   Last Modified by: ksiric
   Last Modified: 2026-06-05 09:47:43
   ---------------------------------------------------------------------
   Description:
       
   ---------------------------------------------------------------------
   License: 
   Company: 
   Version: 0.1.0
 ======================================================================
                                                                       */

#include "CypherEngine/CypherRender/CypherRender_Draw.h"

namespace cypher::engine::render
{

cypher_render_error_code_t CypherRender_DrawItem( const cypher_render_draw_item_t &item, const cypher_render_camera_t &camera )
{
    if ( item.mesh == nullptr ) {
        return cypher_render_error_code_t::ERR_INVALID_FUNC_PARAMETER; 
    }
    
    if ( item.shader == nullptr ) {
        return cypher_render_error_code_t::ERR_INVALID_FUNC_PARAMETER;
    }

    cypher_render_error_code_t result = CypherRender_ShaderBind( *item.shader );
    if ( result != cypher_render_error_code_t::OK ) {
        return result;
    }
    
    result = CypherRender_ShaderSetMat4( *item.shader, "u_model", item.model_matrix );
    if ( result != cypher_render_error_code_t::OK ) {
        return result;
    }         
    
    result = CypherRender_ShaderSetMat4( *item.shader, "u_view", camera.view );
    if ( result != cypher_render_error_code_t::OK ) {
        return result;
    }         
    
    result = CypherRender_ShaderSetMat4( *item.shader, "u_projection", camera.projection );
    if ( result != cypher_render_error_code_t::OK ) {
        return result;
    }         
    
    return CypherRender_MeshDraw( *item.mesh );   
}

void CypherRender_DrawListInit( cypher_render_draw_list_t &draw_list, cypher_render_draw_item_t *items, common::u32 item_capacity )
{
    draw_list.items = items;
    draw_list.item_count = 0u;
    draw_list.item_capacity = item_capacity;

    return ;
}

void CypherRender_DrawListClear( cypher_render_draw_list_t &draw_list )
{
    draw_list.item_count = 0u;

    return ;
}

cypher_render_error_code_t CypherRender_DrawListSubmit( cypher_render_draw_list_t &draw_list, const cypher_render_draw_item_t &item )
{
    if ( draw_list.items == nullptr || draw_list.item_capacity == 0u ) {
        return cypher_render_error_code_t::ERR_INVALID_FUNC_PARAMETER;
    }

    if ( item.mesh == nullptr || item.shader == nullptr ) {
        return cypher_render_error_code_t::ERR_INVALID_FUNC_PARAMETER;
    }

    if ( draw_list.item_count >= draw_list.item_capacity ) {
        return cypher_render_error_code_t::ERR_DRAW_LIST_FULL;
    }

    draw_list.items[draw_list.item_count] = item;
    ++draw_list.item_count;

    return cypher_render_error_code_t::OK;
}

cypher_render_error_code_t CypherRender_DrawListDraw( const cypher_render_draw_list_t &draw_list, const cypher_render_camera_t &camera )
{
    if ( draw_list.item_count == 0u ) {
        return cypher_render_error_code_t::OK;
    }

    if ( draw_list.items == nullptr ) {
        return cypher_render_error_code_t::ERR_INVALID_FUNC_PARAMETER;
    }

    for ( common::u32 i = 0u; i < draw_list.item_count; ++i ) {
        const cypher_render_error_code_t result = CypherRender_DrawItem( draw_list.items[i], camera );

        if ( result != cypher_render_error_code_t::OK ) {
            return result;
        }
    }

    return cypher_render_error_code_t::OK;
}
    
}       // namespace cypher::engine::render

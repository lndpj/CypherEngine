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
#include "CypherEngine/CypherLog/CypherLog.h"

namespace cypher::engine::render
{

error_code_t CypherRender_DrawItem( const draw_item_t &item, const camera_t &camera )
{
    if ( item.mesh == nullptr ) {
        LOG_ERROR( log::channel_t::RENDER, "draw item failed: mesh is null." );
        return error_code_t::ERR_INVALID_FUNC_PARAMETER; 
    }
    
    if ( item.shader == nullptr ) {
        LOG_ERROR( log::channel_t::RENDER, "draw item failed: shader is null." );
        return error_code_t::ERR_INVALID_FUNC_PARAMETER;
    }

    error_code_t result = CypherRender_ShaderBind( *item.shader );
    if ( result != error_code_t::OK ) {
        LOG_ERROR( log::channel_t::RENDER, "draw item failed: shader bind failed: %s.", CypherRender_ErrorDesc( result ) );
        return result;
    }
    
    result = CypherRender_ShaderSetMat4( *item.shader, "u_model", item.model_matrix );
    if ( result != error_code_t::OK ) {
        LOG_ERROR( log::channel_t::RENDER, "draw item failed: setting u_model failed: %s.", CypherRender_ErrorDesc( result ) );
        return result;
    }         
    
    result = CypherRender_ShaderSetMat4( *item.shader, "u_view", camera.view );
    if ( result != error_code_t::OK ) {
        LOG_ERROR( log::channel_t::RENDER, "draw item failed: setting u_view failed: %s.", CypherRender_ErrorDesc( result ) );
        return result;
    }         
    
    result = CypherRender_ShaderSetMat4( *item.shader, "u_projection", camera.projection );
    if ( result != error_code_t::OK ) {
        LOG_ERROR( log::channel_t::RENDER, "draw item failed: setting u_projection failed: %s.", CypherRender_ErrorDesc( result ) );
        return result;
    }         
    
    return CypherRender_MeshDraw( *item.mesh );   
}

void CypherRender_DrawListInit( draw_list_t &draw_list, draw_item_t *items, common::u32 item_capacity )
{
    draw_list.items = items;
    draw_list.item_count = 0u;
    draw_list.item_capacity = item_capacity;

    LOG_DEBUG( log::channel_t::RENDER, "draw list initialized: capacity=%u.", item_capacity );

    return ;
}

void CypherRender_DrawListClear( draw_list_t &draw_list )
{
    draw_list.item_count = 0u;

    return ;
}

error_code_t CypherRender_DrawListSubmit( draw_list_t &draw_list, const draw_item_t &item )
{
    if ( draw_list.items == nullptr || draw_list.item_capacity == 0u ) {
        LOG_ERROR( log::channel_t::RENDER, "draw list submit failed: draw list storage is invalid." );
        return error_code_t::ERR_INVALID_FUNC_PARAMETER;
    }

    if ( item.mesh == nullptr || item.shader == nullptr ) {
        LOG_ERROR( log::channel_t::RENDER, "draw list submit failed: invalid draw item mesh=%p shader=%p.", static_cast<void *>( item.mesh ), static_cast<void *>( item.shader ) );
        return error_code_t::ERR_INVALID_FUNC_PARAMETER;
    }

    if ( draw_list.item_count >= draw_list.item_capacity ) {
        LOG_ERROR( log::channel_t::RENDER, "draw list submit failed: list full (%u/%u).", draw_list.item_count, draw_list.item_capacity );
        return error_code_t::ERR_DRAW_LIST_FULL;
    }

    draw_list.items[draw_list.item_count] = item;
    ++draw_list.item_count;

    return error_code_t::OK;
}

error_code_t CypherRender_DrawListDraw( const draw_list_t &draw_list, const camera_t &camera )
{
    if ( draw_list.item_count == 0u ) {
        return error_code_t::OK;
    }

    if ( draw_list.items == nullptr ) {
        LOG_ERROR( log::channel_t::RENDER, "draw list draw failed: draw list storage is invalid." );
        return error_code_t::ERR_INVALID_FUNC_PARAMETER;
    }

    for ( common::u32 i = 0u; i < draw_list.item_count; ++i ) {
        const error_code_t result = CypherRender_DrawItem( draw_list.items[i], camera );

        if ( result != error_code_t::OK ) {
            LOG_ERROR( log::channel_t::RENDER, "draw list draw failed: item=%u error=%s.", i, CypherRender_ErrorDesc( result ) );
            return result;
        }
    }

    return error_code_t::OK;
}
    
}       // namespace cypher::engine::render

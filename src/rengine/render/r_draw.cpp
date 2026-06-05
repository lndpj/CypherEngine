/*======================================================================
   File: r_draw.cpp
   Project: REAP
   Author: ksiric <email@example.com>
   Created: 2026-06-04 20:27:26
   Last Modified by: ksiric
   Last Modified: 2026-06-05 08:57:15
   ---------------------------------------------------------------------
   Description:
       
   ---------------------------------------------------------------------
   License: 
   Company: 
   Version: 0.1.0
 ======================================================================
                                                                       */

#include "rengine/render/r_draw.h"

namespace reap::rengine::render
{

r_error_code_t R_DrawItem( const r_draw_item_t &item, const r_camera_t &camera )
{
    if ( item.mesh == nullptr ) {
        return r_error_code_t::ERR_INVALID_FUNC_PARAMETER; 
    }
    
    if ( item.shader == nullptr ) {
        return r_error_code_t::ERR_INVALID_FUNC_PARAMETER;
    }

    r_error_code_t result = R_ShaderBind( *item.shader );
    if ( result != r_error_code_t::OK ) {
        return result;
    }
    
    result = R_ShaderSetMat4( *item.shader, "u_model", item.model_matrix );
    if ( result != r_error_code_t::OK ) {
        return result;
    }         
    
    result = R_ShaderSetMat4( *item.shader, "u_view", camera.view );
    if ( result != r_error_code_t::OK ) {
        return result;
    }         
    
    result = R_ShaderSetMat4( *item.shader, "u_projection", camera.projection );
    if ( result != r_error_code_t::OK ) {
        return result;
    }         
    
    return R_MeshDraw( *item.mesh );   
}

void R_DrawListClear( r_draw_list_t &draw_list )
{
    draw_list.item_count = 0u;

    return ;
}

r_error_code_t R_DrawListSubmit( r_draw_list_t &draw_list, const r_draw_item_t &item )
{
    if ( item.mesh == nullptr || item.shader == nullptr ) {
        return r_error_code_t::ERR_INVALID_FUNC_PARAMETER;
    }

    if ( draw_list.item_count >= R_DRAW_ITEMS_LIST_MAX ) {
        return r_error_code_t::ERR_DRAW_LIST_FULL;
    }

    draw_list.items[draw_list.item_count] = item;
    ++draw_list.item_count;

    return r_error_code_t::OK;
}

r_error_code_t R_DrawListDraw( const r_draw_list_t &draw_list, const r_camera_t &camera )
{
    for ( rcommon::u32 i = 0u; i < draw_list.item_count; ++i ) {
        const r_error_code_t result = R_DrawItem( draw_list.items[i], camera );

        if ( result != r_error_code_t::OK ) {
            return result;
        }
    }

    return r_error_code_t::OK;
}
    
}       // namespace reap::rengine::render

/*======================================================================
   File: r_draw.cpp
   Project: REAP
   Author: ksiric <email@example.com>
   Created: 2026-06-04 20:27:26
   Last Modified by: ksiric
   Last Modified: 2026-06-05 08:54:52
   ---------------------------------------------------------------------
   Description:
       
   ---------------------------------------------------------------------
   License: 
   Company: 
   Version: 0.1.0
 ======================================================================
                                                                       */

#include "rengine/render/r_draw.h"
#include "rengine/math/math_mat.h"

namespace reap::rengine::render
{

r_error_code_t R_DrawObject( const r_draw_item_t &item, const r_camera_t &camera )
{
    if ( item.mesh == nullptr ) {
        return r_error_code_t::ERR_INVALID_FUNC_PARAMETER; 
    }
    
    if ( item.shader == nullptr ) {
        return r_error_code_t::ERR_INVALID_FUNC_PARAMETER;
    }
    
    const math::mat4_t model = math::Math_Mat4TranslationRotationScale( 
                               item.position,
                               item.orientation,
                               item.scale );
    r_error_code_t result = R_ShaderBind( *item.shader );
    if ( result != r_error_code_t::OK ) {
        return result;
    }
    
    result = R_ShaderSetMat4( *item.shader, "u_model", model );
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
    
}       // namespace reap::rengine::render

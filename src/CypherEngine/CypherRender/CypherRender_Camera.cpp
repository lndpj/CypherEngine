/*======================================================================
   File: r_camera.cpp
   Project: CypherEngine
   Author: ksiric <email@example.com>
   Created: 2026-06-02 22:45:37
   Last Modified by: ksiric
   Last Modified: 2026-06-03 13:16:08
   ---------------------------------------------------------------------
   Description:
       
   ---------------------------------------------------------------------
   License: 
   Company: 
   Version: 0.1.0
 ======================================================================
                                                                       */

#include "CypherEngine/CypherRender/CypherRender_Camera.h"
#include "CypherEngine/CypherMath/CypherMath_Frustum.h"
#include "CypherEngine/CypherMath/CypherMath_Mat.h"
#include "CypherEngine/CypherMath/CypherMath_Quat.h"

namespace cypher::engine::render
{

void CypherRender_CameraInit( cypher_render_camera_t &camera, const cypher_render_camera_desc_t &camera_desc )
{
    camera = cypher_render_camera_t{};
    
    camera.camera_desc = camera_desc;
    camera.position = math::vec3_t{};
    camera.orientation = math::CypherMath_QuatIdentity();    
    
    CypherRender_CameraUpdateMatrices( camera );
}

void CypherRender_CameraUpdateMatrices( cypher_render_camera_t &camera )
{
    camera.orientation = math::CypherMath_QuatNormalize( camera.orientation );
    
    const math::vec3_t forward  = math::CypherMath_QuatForwardVec3( camera.orientation );
    const math::vec3_t up       = math::CypherMath_QuatUpVec3( camera.orientation );
    
    // we do not need right basis vector because the LookAt matrix func builds it already internally
    
    const math::vec3_t target   = math::CypherMath_Vec3Add( camera.position, forward );
    
    camera.view = math::CypherMath_Mat4LookAt( camera.position, target, up );
    camera.projection = math::CypherMath_Mat4Perspective( camera.camera_desc.fov_y_radians,
                                                    camera.camera_desc.aspect_ratio,
                                                    camera.camera_desc.near_z,
                                                    camera.camera_desc.far_z );
    camera.projection_view = math::CypherMath_Mat4Multiply( camera.projection, camera.view );
    camera.frustum = math::CypherMath_FrustumFromProjectionView( camera.projection_view );
}

void CypherRender_CameraSetPerspective( cypher_render_camera_t &camera, common::f32 fov_y_radians, common::f32 aspect_ratio, common::f32 near_z, common::f32 far_z )
{
    camera.camera_desc.fov_y_radians = fov_y_radians;
    camera.camera_desc.aspect_ratio = aspect_ratio;
    camera.camera_desc.near_z = near_z;
    camera.camera_desc.far_z = far_z;
    
    CypherRender_CameraUpdateMatrices( camera );
}

void CypherRender_CameraSetTransform( cypher_render_camera_t &camera, const math::vec3_t &position, const math::quat_t &orientation )
{
    camera.position = position;
    camera.orientation = math::CypherMath_QuatNormalize( orientation );
    
    CypherRender_CameraUpdateMatrices( camera );
}

void CypherRender_CameraSetPosition( cypher_render_camera_t &camera, const math::vec3_t &position )
{
    camera.position = position;
    
    CypherRender_CameraUpdateMatrices( camera );
}

void CypherRender_CameraSetOrientation( cypher_render_camera_t &camera, const math::quat_t &orientation )
{   
    camera.orientation = math::CypherMath_QuatNormalize( orientation );
          
    CypherRender_CameraUpdateMatrices( camera );
}

void CypherRender_CameraSetPerspectiveMode( cypher_render_camera_t &camera, cypher_render_camera_projection_mode_t &mode )
{
    camera.camera_desc.camera_projection_mode = mode;
    
    CypherRender_CameraUpdateMatrices( camera );
}

}       // namespace cypher::engine::render

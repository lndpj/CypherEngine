/*======================================================================
   File: r_camera.cpp
   Project: REAP
   Author: ksiric <email@example.com>
   Created: 2026-06-02 22:45:37
   Last Modified by: ksiric
   Last Modified: 2026-06-03 10:38:15
   ---------------------------------------------------------------------
   Description:
       
   ---------------------------------------------------------------------
   License: 
   Company: 
   Version: 0.1.0
 ======================================================================
                                                                       */

#include "rengine/render/r_camera.h"
#include "rengine/math/math_frustum.h"
#include "rengine/math/math_mat.h"
#include "rengine/math/math_quat.h"

namespace reap::rengine::render
{

void R_CameraInit( r_camera_t &camera, const r_camera_desc_t &camera_desc )
{
    camera = r_camera_t{};
    
    camera.camera_desc = camera_desc;
    camera.position = math::vec3_t{};
    camera.orientation = math::Math_QuatIdentity();    
    
    R_CameraUpdateMatrices( camera );
}

void R_CameraUpdateMatrices( r_camera_t &camera )
{
    camera.orientation = math::Math_QuatNormalize( camera.orientation );
    
    const math::vec3_t forward  = math::Math_QuatForwardVec3( camera.orientation );
    const math::vec3_t up       = math::Math_QuatUpVec3( camera.orientation );
    
    // we do not need right basis vector because the LookAt matrix func builds it already internally
    
    const math::vec3_t target   = math::Math_Vec3Add( camera.position, forward );
    
    camera.view = math::Math_Mat4LookAt( camera.position, target, up );
    camera.projection = math::Math_Mat4Perspective( camera.camera_desc.fov_y_radians,
                                                    camera.camera_desc.aspect_ratio,
                                                    camera.camera_desc.near_z,
                                                    camera.camera_desc.far_z );
    camera.projection_view = math::Math_Mat4Multiply( camera.projection, camera.view );
    camera.frustum = math::Math_FrustumFromProjectionView( camera.projection_view );
}
 
 
 
 
    
}       // namespace reap::rengine::render

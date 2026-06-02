/*======================================================================
   File: r_camera.cpp
   Project: REAP
   Author: ksiric <email@example.com>
   Created: 2026-06-02 22:45:37
   Last Modified by: ksiric
   Last Modified: 2026-06-02 22:48:14
   ---------------------------------------------------------------------
   Description:
       
   ---------------------------------------------------------------------
   License: 
   Company: 
   Version: 0.1.0
 ======================================================================
                                                                       */

#include "rengine/render/r_camera.h"
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


    
}       // namespace reap::rengine::render

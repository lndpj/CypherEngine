#pragma once

#include "rengine/math/math_vec.h"
#include "rengine/render/r_config.h"

namespace reap::rengine::render
{
    
enum r_camera_projection_mode_t {
    PERSPECTIVE,
    ORTOGRAPHIC
};  

struct r_camera_desc_t {
    r_camera_projection_mode_t camera_projection_mode{ r_camera_projection_mode_t::PERSPECTIVE };
    rcommon::f32 fov_y_radians{ R_DEFAULT_FOV_Y_RADIANS };
    rcommon::f32 aspect_ratio{ R_DEFAULT_ASPECT_RATIO };
    rcommon::f32 near_z{ R_DEFAULT_NEAR_Z };
    rcommon::f32 far_z{ R_DEFAULT_FAR_Z };
};

struct r_camera_t {
    math::vec3_t position{};
    math::quat_t orientation{};
    
    r_camera_desc_t camera_desc{};
    
    math::mat4_t view{};
    math::mat4_t projection{};
    math::mat4_t projection_view{};
    math::frustum_t frustum{};
};
 
void R_CameraInit( r_camera_t &camera, const r_camera_desc_t &camera_desc );
    
    
    
    
}       // namespace reap::rengine::render

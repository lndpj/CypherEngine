#pragma once

#include "CypherEngine/CypherMath/CypherMath_Vec.h"
#include "CypherEngine/CypherRender/CypherRender_Config.h"

namespace cypher::engine::render
{
    
enum cypher_render_camera_projection_mode_t {
    PERSPECTIVE,
    ORTOGRAPHIC
};  

struct cypher_render_camera_desc_t {
    cypher_render_camera_projection_mode_t camera_projection_mode{ cypher_render_camera_projection_mode_t::PERSPECTIVE };
    common::f32 fov_y_radians{ CYPHER_RENDER_DEFAULT_FOV_Y_RADIANS };
    common::f32 aspect_ratio{ CYPHER_RENDER_DEFAULT_ASPECT_RATIO };
    common::f32 near_z{ CYPHER_RENDER_DEFAULT_NEAR_Z };
    common::f32 far_z{ CYPHER_RENDER_DEFAULT_FAR_Z };
};

struct cypher_render_camera_t {
    math::vec3_t position{};
    math::quat_t orientation{};
    
    cypher_render_camera_desc_t camera_desc{};
    
    math::mat4_t view{};
    math::mat4_t projection{};
    math::mat4_t projection_view{};
    math::frustum_t frustum{};
};
 
void CypherRender_CameraInit( cypher_render_camera_t &camera, const cypher_render_camera_desc_t &camera_desc );
    
void CypherRender_CameraUpdateMatrices( cypher_render_camera_t &camera );    

void CypherRender_CameraSetPerspective( cypher_render_camera_t &camera, common::f32 fov_y_radians, common::f32 aspect_ration, common::f32 near_z, common::f32 far_z );

void CypherRender_CameraSetTransform( cypher_render_camera_t &camera, const math::vec3_t &position, const math::quat_t &orientation );

void CypherRender_CameraSetPosition( cypher_render_camera_t &camera, const math::vec3_t &position );

void CypherRender_CameraSetOrientation( cypher_render_camera_t &camera, const math::quat_t &orientation );

void CypherRender_CameraSetPerspectiveMode( cypher_render_camera_t &camera, cypher_render_camera_projection_mode_t &mode );
    
}       // namespace cypher::engine::render

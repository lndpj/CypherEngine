#ifndef CYPHER_ENGINE_RENDER_CAMERA_H
#define CYPHER_ENGINE_RENDER_CAMERA_H

#pragma once

#include "CypherEngine/CypherMath/CypherMath_Vec.h"
#include "CypherEngine/CypherRender/CypherRender_Config.h"

namespace cypher::engine::render
{
    
enum camera_projection_mode_t {
    PERSPECTIVE,
    ORTOGRAPHIC
};  

struct camera_desc_t {
    camera_projection_mode_t camera_projection_mode{ camera_projection_mode_t::PERSPECTIVE };
    common::f32 fov_y_radians{ CYPHER_RENDER_DEFAULT_FOV_Y_RADIANS };
    common::f32 aspect_ratio{ CYPHER_RENDER_DEFAULT_ASPECT_RATIO };
    common::f32 near_z{ CYPHER_RENDER_DEFAULT_NEAR_Z };
    common::f32 far_z{ CYPHER_RENDER_DEFAULT_FAR_Z };
};

struct camera_t {
    math::vec3_t position{};
    math::quat_t orientation{};
    
    camera_desc_t camera_desc{};
    
    math::mat4_t view{};
    math::mat4_t projection{};
    math::mat4_t projection_view{};
    math::frustum_t frustum{};
};
 
void CypherRender_CameraInit( camera_t &camera, const camera_desc_t &camera_desc );
    
void CypherRender_CameraUpdateMatrices( camera_t &camera );    

void CypherRender_CameraSetPerspective( camera_t &camera, common::f32 fov_y_radians, common::f32 aspect_ration, common::f32 near_z, common::f32 far_z );

void CypherRender_CameraSetTransform( camera_t &camera, const math::vec3_t &position, const math::quat_t &orientation );

void CypherRender_CameraSetPosition( camera_t &camera, const math::vec3_t &position );

void CypherRender_CameraSetOrientation( camera_t &camera, const math::quat_t &orientation );

void CypherRender_CameraSetPerspectiveMode( camera_t &camera, camera_projection_mode_t &mode );
    
}       // namespace cypher::engine::render

#endif // CYPHER_ENGINE_RENDER_CAMERA_H

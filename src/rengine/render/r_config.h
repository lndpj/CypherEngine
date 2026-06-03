#pragma once

#include "rengine/math/math_types.h"
#include "rengine/rcommon/com_main.h"

namespace reap::rengine::render
{

// DEFAULT CONSTANTS FOR THE RENDERER, LATER IN THE CONFIG DEFINED
constexpr rcommon::f32 R_DEFAULT_FOV_Y_RADIANS      = 70.0f * math::MATH_DEG2RAD_F;
constexpr rcommon::f32 R_DEFAULT_NEAR_Z             = 0.1f;
constexpr rcommon::f32 R_DEFAULT_FAR_Z              = 4096.0f;

constexpr rcommon::f32 R_DEFAULT_VIEWPORT_WIDTH     = 1280u;
constexpr rcommon::f32 R_DEFAULT_VIEWPORT_HEIGHT    = 720u;
constexpr rcommon::f32 R_DEFAULT_ASPECT_RATIO       = ( R_DEFAULT_VIEWPORT_WIDTH / R_DEFAULT_VIEWPORT_HEIGHT );
    
}       // namespace reap::rengine::render

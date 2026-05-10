#pragma once

#include "rengine/rcommon/com_main.h"

namespace reap::rengine::math
{ 
    
/*
================
Vector Types

Small POD math types used by renderer, map, collision and gameplay code.
================
*/
struct vec2_t {
    rcommon::f32 x{};
    rcommon::f32 y{};
};

struct vec3_t {
    rcommon::f32 x{};
    rcommon::f32 y{};
    rcommon::f32 z{};
};

struct vec4_t {
    rcommon::f32 x{};
    rcommon::f32 y{};
    rcommon::f32 z{};
    rcommon::f32 w{};
};

struct mat4_t {
    rcommon::f32 m[16]{};
};

/*
================
plane_t / bounds_t

Plane is the base primitive for BSP, frustum and collision work.
Bounds is an axis-aligned box used for visibility, collision and asset limits.
================
*/
struct plane_t {
    vec3_t normal{};
    rcommon::f32 dist{};
};

struct bounds_t {
    vec3_t mins{};
    vec3_t maxs{};
};

}       // namespace reap::rengine::math

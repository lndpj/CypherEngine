#pragma once

#include "rengine/rcommon/com_main.h"

namespace reap::rengine::math
{ 
/*
================
Math Constants

Typed math constants used by renderer, collision, BSP, animation and gameplay.
================
*/
constexpr rcommon::f32 MATH_PI_F        = 3.14159265358979323846f;
constexpr rcommon::f32 MATH_TAU_F       = 6.28318530717958647692f;
constexpr rcommon::f32 MATH_HALF_PI_F   = MATH_PI_F * 0.5f;
constexpr rcommon::f32 MATH_DEG2RAD_F   = MATH_PI_F / 180.0f;
constexpr rcommon::f32 MATH_RAD2DEG_F   = 180.0f / MATH_PI_F;
constexpr rcommon::f32 MATH_EPSILON_F   = 1.0e-6f;

constexpr rcommon::f64 MATH_PI_D        = 3.14159265358979323846264338327950288;
constexpr rcommon::f64 MATH_EPSILON_D   = 1.0e-12; 
    
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
Packed Vector Types

Used for file formats, BSP data, compact network data, and other packed storage.
Do not use these for normal runtime movement or rendering math.
================
*/

struct vec3_s_t {
    rcommon::i16 x{};  
    rcommon::i16 y{};  
    rcommon::i16 z{};  
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

static_assert( sizeof( vec3_t ) == sizeof( rcommon::f32 ) * 3u, "vec3_t must stay tightly packed" );
static_assert( sizeof( vec3_s_t ) == sizeof( rcommon::i16 ) * 3u, "vec3_s_t must stay tightly packed" );
static_assert( sizeof( mat4_t ) == sizeof( rcommon::f32 ) * 16u, "mat4_t must stay tightly packed" );

}       // namespace reap::rengine::math

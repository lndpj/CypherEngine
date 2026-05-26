#pragma once

#include "rengine/math/math_types.h"

namespace reap::rengine::math
{

vec3_t Math_RayPointAt( const ray_t &ray, rcommon::f32 t_units );

bool Math_RayIntersectsPlane( const ray_t &ray, const plane_t &plane, rcommon::f32 &out_t_units );

bool Math_RayIntersectsBounds( const ray_t &ray, const bounds_t &bounds, rcommon::f32 &out_tmin, rcommon::f32 &out_tmax );
    
}       // namespace reap::rengine::math

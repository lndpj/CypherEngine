#pragma once

#include "rengine/math/math_types.h"

namespace reap::rengine::math
{

bounds_t Math_BoundsClear();

bounds_t Math_BoundsFromPoint( const vec3_t &v );

void Math_BoundsAddPoint( bounds_t &bounds, const vec3_t &point );

vec3_t Math_BoundsCenter( const bounds_t &bounds );

vec3_t Math_BoundsSize( const bounds_t &bounds );

bool Math_BoundsContainsPoint( const bounds_t &bounds, const vec3_t &point );

bool Math_BoundsIntersects( const bounds_t &b1, const bounds_t &b2 );
    
}       // namespace reap::rengine::math

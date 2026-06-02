#pragma once

#include "rengine/math/math_types.h"

namespace reap::rengine::math
{

plane_t Math_PlaneFromPointNormal( const vec3_t &point, const vec3_t &normal );

plane_t Math_PlaneFromPoints( const vec3_t &p0, const vec3_t &p1, const vec3_t &p2 );

rcommon::f32 Math_PlaneDistance( const plane_t &plane, const vec3_t &v );

bool Math_PlanePointFront( const plane_t &plane, const vec3_t &v );

bool Math_PlanePointBack( const plane_t &plane, const vec3_t &v );

bool Math_PlanePointOn( const plane_t &plane, const vec3_t &v, rcommon::f32 epsilon );
    
}       // namespace reap::rengine::math

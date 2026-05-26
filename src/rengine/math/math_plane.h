#pragma once

#include "rengine/math/math_types.h"
#include "rengine/rcommon/com_main.h"

namespace reap::rengine::math
{

rcommon::f32 Math_PlaneDistance( const plane_t &plane, const vec3_t &v );

bool Math_PlanePointFront( const plane_t &plane, const vec3_t &v );

bool Math_PlanePointBack( const plane_t &plane, const vec3_t &v );

bool Math_PlanePointOn( const plane_t &plane, const vec3_t &v, rcommon::f32 epsilon );
    
}       // namespace reap::rengine::math

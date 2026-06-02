#pragma once

#include "rengine/math/math_types.h"

namespace reap::rengine::math
{
 
frustum_t Math_FrustumFromProjectionView( const mat4_t &projection_view );
    
bool Math_FrustumContainsPoint( const frustum_t &frustum, const vec3_t &point );

bool Math_FrustumIntersectsBounds( const frustum_t &frustum, const bounds_t &bounds );  
    
}       // namespace reap::rengine::math

/*======================================================================
   File: math_bounds.cpp
   Project: rengine
   Author: ksiric <email@example.com>
   Created: 2026-05-25 02:41:52
   Last Modified by: ksiric
   Last Modified: 2026-05-26 18:57:17
   ---------------------------------------------------------------------
   Description:
       
   ---------------------------------------------------------------------
   License: 
   Company: 
   Version: 0.1.0
 ======================================================================
                                                                       */

#include "rengine/math/math_bounds.h"
#include "rengine/math/math_vec.h"

#include <limits>

namespace reap::rengine::math
{
    
bounds_t Math_BoundsClear()
{
    const rcommon::f32 max_value = std::numeric_limits<rcommon::f32>::max();
    
    return bounds_t{
        vec3_t{ max_value, max_value, max_value },
        vec3_t{ -max_value, -max_value, -max_value }
    };
}

bounds_t Math_BoundsFromPoint( const vec3_t &v )
{
    return bounds_t{ v, v };
}

void Math_BoundsAddPoint( bounds_t &bounds, const vec3_t &point )
{
    bounds.mins = Math_Vec3Min( bounds.mins , point );
    bounds.maxs = Math_Vec3Max( bounds.maxs, point );
}

vec3_t Math_BoundsCenter( const bounds_t &bounds )
{
    return Math_Vec3Scale( Math_Vec3Add( bounds.mins, bounds.maxs ), 0.5f );
}

vec3_t Math_BoundsSize( const bounds_t &bounds )
{
    return Math_Vec3Sub( bounds.maxs, bounds.mins );
}

bool Math_BoundsContainsPoint( const bounds_t &bounds, const vec3_t &point )
{
    return point.x >= bounds.mins.x && point.x <= bounds.maxs.x &&
           point.y >= bounds.mins.y && point.y <= bounds.maxs.y &&
           point.z >= bounds.mins.z && point.z <= bounds.maxs.z;
}

bool Math_BoundsIntersects( const bounds_t &b1, const bounds_t &b2 )
{
    if ( b1.maxs.x < b2.mins.x || b1.mins.x > b2.maxs.x ) {
        return false;
    }
    if ( b1.maxs.y < b2.mins.y || b1.mins.y > b2.maxs.y ) {
        return false;
    }
    if ( b1.maxs.z < b2.mins.z || b1.mins.z > b2.maxs.z ) {
        return false;
    }
    return true;
}

}       // namespace reap::rengine::math   

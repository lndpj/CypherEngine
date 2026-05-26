/*======================================================================
   File: math_plane.cpp
   Project: rengine
   Author: ksiric <email@example.com>
   Created: 2026-05-25 02:41:48
   Last Modified by: ksiric
   Last Modified: 2026-05-25 03:01:37
   ---------------------------------------------------------------------
   Description:
       
   ---------------------------------------------------------------------
   License: 
   Company: 
   Version: 0.1.0
 ======================================================================
                                                                       */

#include "rengine/math/math_plane.h"
#include "rengine/math/math_vec.h"

namespace reap::rengine::math
{

rcommon::f32 Math_PlaneDistance( const plane_t &plane, const vec3_t &v )
{
    /*
     * Corresponds to either giving a positive or negative value.
     * Positive value corresponds to a point being in front of the plane.
     * Negative value corresponds to a point being behind the plane.
     */
    return Math_Vec3Dot( plane.normal, v ) - plane.dist;
}

bool Math_PlanePointFront( const plane_t &plane, const vec3_t &v )
{
    /*
    const rcommon::f32 vec_dot = Math_Vec3Dot( plane.normal , v );
    if ( ( vec_dot - plane.dist ) < 0.0f ) {
        return false;
    } 
    return true;
    */
    return Math_PlaneDistance( plane, v ) > MATH_EPSILON_F;
}

bool Math_PlanePointBack( const plane_t &plane, const vec3_t &v )
{
    /*
    const rcommon::f32 vec_dot = Math_Vec3Dot( plane.normal , v );
    if ( ( vec_dot - plane.dist ) > 0.0f ) {
        return false;
    } 
    return true;
    */
    return Math_PlaneDistance( plane, v ) < -MATH_EPSILON_F;
}

bool Math_PlanePointOn( const plane_t &plane, const vec3_t &v, rcommon::f32 epsilon )
{
    const rcommon::f32 distance = Math_PlaneDistance( plane, v );
    
    return distance >= -epsilon && distance <= epsilon;
}
    
}       // namespace reap::rengine::math

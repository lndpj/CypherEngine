/*======================================================================
   File: math_vec.cpp
   Project: REAP
   Author: ksiric <email@example.com>
   Created: 2026-05-11 22:30:00
   Last Modified by: ksiric
   Last Modified: 2026-05-19 15:03:17
   ---------------------------------------------------------------------
   Description:
       
   ---------------------------------------------------------------------
   License: 
   Company: 
   Version: 0.1.0
 ======================================================================
                                                                       */

#include "rengine/math/math_vec.h"

#include <cmath>    // std::sqrt / std::fabs

namespace reap::rengine::math
{

/*
==================
Vec2
==================
*/
rcommon::f32 Math_Vec2Length( const vec2_t &v ) {
    return std::sqrt( Math_Vec2Dot( v, v ) );
}

rcommon::f32 Math_Vec2Distance( const vec2_t &v1, const vec2_t &v2 ) {
    return Math_Vec2Length( Math_Vec2Sub( v1, v2 ) );   
}

vec2_t Math_Vec2Normalize(const vec2_t &v ) {
    const rcommon::f32 vec_len = Math_Vec2Length( v );
    
    // @NOTE: In case the length of the vector is very very small we count it as 0.
    if ( vec_len <= MATH_EPSILON_F ) {
        return vec2_t{};
    }
    
    return Math_Vec2Scale( v, 1.0f / vec_len );
}

rcommon::f32 Math_Vec2NormalizeLength( vec2_t &v ) {
    const rcommon::f32 vec_len = Math_Vec2Length( v );
    
    if ( vec_len <= MATH_EPSILON_F ) {
        v = vec2_t{};
        return 0.0f;
    }
    
    v = Math_Vec2Scale( v, 1.0f / vec_len );
    return vec_len;
}

bool Math_Vec2Near( const vec2_t &v1, const vec2_t &v2, rcommon::f32 epsilon ) {
    return ( std::fabs( v1.x - v2.x ) <= epsilon &&
             std::fabs( v1.y - v2.y ) <= epsilon );
}

vec2_t Math_Vec2Lerp( const vec2_t &v1, const vec2_t &v2, rcommon::f32 t ) {
    
    return vec2_t{
        v1.x + ( v2.x - v1.x ) * t,
        v1.y + ( v2.y - v1.y ) * t
    };
}

vec2_t Math_Vec2Min( const vec2_t &v1, const vec2_t &v2 ) {
    return vec2_t{
        ( v1.x < v2.x ) ? v1.x : v2.x,
        ( v1.y < v2.y ) ? v1.y : v2.y
    };
}

vec2_t Math_Vec2Max( const vec2_t &v1, const vec2_t &v2 ) {
    return vec2_t{
        ( v1.x > v2.x ) ? v1.x : v2.x,
        ( v1.y > v2.y ) ? v1.y : v2.y
    };
}

/*
==================
Vec3
==================
*/

/*
 * @NOTE(Karlo 19.5.26) Adding all operations and vector3 construcvtors
 *                      Constructors first and then the operations necessary.
 */









vec3_t Math_Vec3Cross( const vec3_t &v1, const vec3_t &v2 )
{
    return vec3_t{
        v1.y * v2.z - v1.z * v2.y,
        v1.z * v2.x - v1.x * v2.z,
        v1.x * v2.y - v1.y * v2.x
    };
}

rcommon::f32 Math_Vec3Length( const vec3_t &v ) 
{
    return std::sqrt( Math_Vec3Dot( v, v ) );
}

vec3_t Math_Vec3Normalize( const vec3_t &v ) {
    const rcommon::f32 vec_len = Math_Vec3Length( v );
    
    if ( vec_len <= MATH_EPSILON_F ) {
        return vec3_t{};
    }
    
    return Math_Vec3Scale( v, 1.0f / vec_len );
}

}       // namespace reap::rengine::math


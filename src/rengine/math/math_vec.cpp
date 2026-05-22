/*======================================================================
   File: math_vec.cpp
   Project: REAP
   Author: ksiric <email@example.com>
   Created: 2026-05-11 22:30:00
   Last Modified by: ksiric
   Last Modified: 2026-05-22 12:21:44
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

rcommon::f32 Math_Vec3Distance( const vec3_t &v1, const vec3_t &v2 ) {
    return Math_Vec3Length( Math_Vec3Sub( v1, v2 ) );   
}



vec3_t Math_Vec3Normalize( const vec3_t &v ) {
    const rcommon::f32 vec_len = Math_Vec3Length( v );
    
    if ( vec_len <= MATH_EPSILON_F ) {
        return vec3_t{};
    }
    
    return Math_Vec3Scale( v, 1.0f / vec_len );
}

rcommon::f32 Math_Vec3NormalizeLength( vec3_t &v ) {
    const rcommon::f32 vec_len = Math_Vec3Length( v );
    
    if ( vec_len <= MATH_EPSILON_F ) {
        v = vec3_t{};
        return 0.0f;
    }
    
    v = Math_Vec3Scale( v, ( 1.0f / vec_len ) );
    
    return vec_len;
}

bool Math_Vec3Near( const vec3_t &v1, const vec3_t &v2, rcommon::f32 epsilon ) {
    return ( std::fabs( v1.x - v2.x ) <= epsilon &&
             std::fabs( v1.y - v2.y ) <= epsilon &&
             std::fabs( v1.z - v2.z ) <= epsilon );
       
}

vec3_t Math_Vec3Lerp( const vec3_t &v1, const vec3_t &v2, rcommon::f32 t ) {
    return vec3_t{
        v1.x - ( v2.x - v1.x ) * t,
        v1.y - ( v2.y - v1.y ) * t,
        v1.z - ( v2.z - v1.z ) * t  
    };
}

vec3_t Math_Vec3Min( const vec3_t &v1, const vec3_t &v2 ) {
    return vec3_t{
        ( v1.x < v2.x ) ? v1.x : v2.x,
        ( v1.y < v2.y ) ? v1.y : v2.y,
        ( v1.z < v2.z ) ? v1.z : v2.z
    };
}

vec3_t Math_Vec3Max( const vec3_t &v1, const vec3_t &v2 ) {
    return vec3_t{
        ( v1.x > v2.x ) ? v1.x : v2.x,
        ( v1.y > v2.y ) ? v1.y : v2.y,
        ( v1.z > v2.z ) ? v1.z : v2.z
    };
}

vec3_t Math_Vec3Reflect( const vec3_t &v, const vec3_t &normal ) {
    rcommon::f32 reflect_dot = Math_Vec3Dot( v, normal );
    return Math_Vec3Sub( v, Math_Vec3Scale( normal, reflect_dot * 2.0f ) );
}

vec3_t Math_Vec3Project( const vec3_t &v, const vec3_t &onto ) {
    rcommon::f32 norm_onto = Math_Vec3LengthSquared( onto );
    if ( norm_onto <= MATH_EPSILON_F ) {
        return vec3_t{};
    }   
    
    rcommon::f32 scale = Math_Vec3Dot( v, onto ) / norm_onto;
    return Math_Vec3Scale( onto, scale );
}

vec3_t Math_Vec3Reject( const vec3_t &v, const vec3_t &from ) {
    return Math_Vec3Sub( v, Math_Vec3Project( v, from ) );
}

/*
==================
Vec4
==================
*/
rcommon::f32 Math_Vec4Length( const vec4_t &v ) {
    return std::sqrt( Math_Vec4Dot( v, v ) );
}

vec4_t Math_Vec4Normalize( const vec4_t &v ) {
    const rcommon::f32 vec_len = Math_Vec4Length( v );
    
    if ( vec_len <= MATH_EPSILON_F ) {
        return vec4_t{};
    }
    return Math_Vec4Scale( v, 1.0f / vec_len );
}

bool Math_Vec4Near( const vec4_t &v1, const vec4_t &v2, rcommon::f32 epsilon ) {
    return ( std::fabs( v1.x - v2.x ) <= epsilon &&
             std::fabs( v1.y - v2.y ) <= epsilon &&
             std::fabs( v1.z - v2.z ) <= epsilon &&
             std::fabs( v1.w - v2.w ) <= epsilon 
             );
}

vec4_t Math_Vec4Lerp( const vec4_t &v1, const vec4_t &v2, rcommon::f32 t ) {
    return vec4_t{
        v1.x - ( v2.x - v1.x ) * t,
        v1.y - ( v2.y - v1.y ) * t,
        v1.z - ( v2.z - v1.z ) * t,
        v1.w - ( v2.w - v1.w ) * t  
    };
}

vec4_t Math_Vec4Min( const vec4_t &v1, const vec4_t &v2 ) {
    return vec4_t{
        ( v1.x < v2.x ) ? v1.x : v2.x,
        ( v1.y < v2.y ) ? v1.y : v2.y,
        ( v1.z < v2.z ) ? v1.z : v2.z,
        ( v1.w < v2.w ) ? v1.w : v2.w
    };
}

vec4_t Math_Vec4Max( const vec4_t &v1, const vec4_t &v2 ) {
    return vec4_t{
        ( v1.x > v2.x ) ? v1.x : v2.x,
        ( v1.y > v2.y ) ? v1.y : v2.y,
        ( v1.z > v2.z ) ? v1.z : v2.z,
        ( v1.w > v2.w ) ? v1.w : v2.w
    };
}


}       // namespace reap::rengine::math




/*======================================================================
   File: math_mat.cpp
   Project: rengine
   Author: ksiric <email@example.com>
   Created: 2026-05-23 11:16:37
   Last Modified by: ksiric
   Last Modified: 2026-05-23 12:22:59
   ---------------------------------------------------------------------
   Description:
       
   ---------------------------------------------------------------------
   License: 
   Company: 
   Version: 0.1.0
 ======================================================================
                                                                       */

#include "rengine/math/math_mat.h"

namespace reap::rengine::math
{

mat4_t Math_Mat4Zero()
{
    return mat4_t{};
}

mat4_t Math_Mat4Identity()
{
    mat4_t result{};
    
    Math_Mat4Set( result, 0u, 0u, 1.0f );
    Math_Mat4Set( result, 1u, 1u, 1.0f );
    Math_Mat4Set( result, 2u, 2u, 1.0f );
    Math_Mat4Set( result, 3u, 3u, 1.0f );
    
    return result;
}

mat4_t Math_Mat4Multiply( const mat4_t &m1, const mat4_t &m2 )
{
    mat4_t result = Math_Mat4Zero();
    
    for ( rcommon::u32 column = 0u; column < 4u; ++column ) {
        for ( rcommon::u32 row = 0u; row < 4u; ++row ) {
            result.m[Math_Mat4Index( column, row )] =
            m1.m[Math_Mat4Index( 0u, row )] * m2.m[Math_Mat4Index( column, 0u )] +
            m1.m[Math_Mat4Index( 1u, row )] * m2.m[Math_Mat4Index( column, 1u )] +
            m1.m[Math_Mat4Index( 2u, row )] * m2.m[Math_Mat4Index( column, 2u )] +
            m1.m[Math_Mat4Index( 3u, row )] * m2.m[Math_Mat4Index( column, 3u )]; 
        }
    }
    return result;
}

mat4_t Math_Mat4Translation( const vec3_t &translation_vector ) 
{
    mat4_t result = Math_Mat4Identity();
    
    result.m[Math_Mat4Index( 3u, 0u )] = translation_vector.x;
    result.m[Math_Mat4Index( 3u, 1u )] = translation_vector.y;
    result.m[Math_Mat4Index( 3u, 2u )] = translation_vector.z;
    
    return result;
}

mat4_t Math_Mat4Scale( const vec3_t &scale ) 
{
    mat4_t result = Math_Mat4Identity();
    
    result.m[Math_Mat4Index( 0u, 0u )] = scale.x;
    result.m[Math_Mat4Index( 1u, 1u )] = scale.y;
    result.m[Math_Mat4Index( 2u, 2u )] = scale.z;
    
    return result;
}



    
}       // namespace reap::rengine::math



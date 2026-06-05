/*======================================================================
   File: math_mat.cpp
   Project: rengine
   Author: ksiric <email@example.com>
   Created: 2026-05-23 11:16:37
   Last Modified by: ksiric
   Last Modified: 2026-06-03 18:01:45
   ---------------------------------------------------------------------
   Description:
       
   ---------------------------------------------------------------------
   License: 
   Company: 
   Version: 0.1.0
 ======================================================================
                                                                       */

#include "CypherEngine/CypherMath/CypherMath_Mat.h"
#include "CypherEngine/CypherMath/CypherMath_Quat.h"
#include "CypherEngine/CypherMath/CypherMath_Vec.h"

#include <cmath>            // std::cos / std::sin etc / std::tan

namespace cypher::engine::math
{

mat4_t CypherMath_Mat4Zero()
{
    return mat4_t{};
}

mat4_t CypherMath_Mat4Identity()
{
    mat4_t result{};
    
    CypherMath_Mat4Set( result, 0u, 0u, 1.0f );
    CypherMath_Mat4Set( result, 1u, 1u, 1.0f );
    CypherMath_Mat4Set( result, 2u, 2u, 1.0f );
    CypherMath_Mat4Set( result, 3u, 3u, 1.0f );
    
    return result;
}

mat4_t CypherMath_Mat4Multiply( const mat4_t &m1, const mat4_t &m2 )
{
    mat4_t result = CypherMath_Mat4Zero();
    
    for ( common::u32 column = 0u; column < 4u; ++column ) {
        for ( common::u32 row = 0u; row < 4u; ++row ) {
            result.m[CypherMath_Mat4Index( column, row )] =
            m1.m[CypherMath_Mat4Index( 0u, row )] * m2.m[CypherMath_Mat4Index( column, 0u )] +
            m1.m[CypherMath_Mat4Index( 1u, row )] * m2.m[CypherMath_Mat4Index( column, 1u )] +
            m1.m[CypherMath_Mat4Index( 2u, row )] * m2.m[CypherMath_Mat4Index( column, 2u )] +
            m1.m[CypherMath_Mat4Index( 3u, row )] * m2.m[CypherMath_Mat4Index( column, 3u )]; 
        }
    }
    return result;
}

mat4_t CypherMath_Mat4Translation( const vec3_t &translation_vector ) 
{
    mat4_t result = CypherMath_Mat4Identity();
    
    result.m[CypherMath_Mat4Index( 3u, 0u )] = translation_vector.x;
    result.m[CypherMath_Mat4Index( 3u, 1u )] = translation_vector.y;
    result.m[CypherMath_Mat4Index( 3u, 2u )] = translation_vector.z;
    
    return result;
}

mat4_t CypherMath_Mat4Scale( const vec3_t &scale ) 
{
    mat4_t result = CypherMath_Mat4Identity();
    
    result.m[CypherMath_Mat4Index( 0u, 0u )] = scale.x;
    result.m[CypherMath_Mat4Index( 1u, 1u )] = scale.y;
    result.m[CypherMath_Mat4Index( 2u, 2u )] = scale.z;
    
    return result;
}

mat4_t CypherMath_Mat4RotateX( const common::f32 radians ) 
{
    mat4_t result = CypherMath_Mat4Identity();
    
    const common::f32 c = std::cos( radians );
    const common::f32 s = std::sin( radians );
    
    result.m[CypherMath_Mat4Index( 1u, 1u )] = c;
    result.m[CypherMath_Mat4Index( 1u, 2u )] = s;
    result.m[CypherMath_Mat4Index( 2u, 1u )] = -s;
    result.m[CypherMath_Mat4Index( 2u, 2u )] = c;
    
    return result;
}

mat4_t CypherMath_Mat4RotateY( const common::f32 radians )
{
    mat4_t result = CypherMath_Mat4Identity();
    
    const common::f32 c = std::cos( radians );
    const common::f32 s = std::sin( radians );
    
    result.m[CypherMath_Mat4Index( 0u, 0u )] = c;
    result.m[CypherMath_Mat4Index( 0u, 2u )] = -s;
    result.m[CypherMath_Mat4Index( 2u, 0u )] = s;
    result.m[CypherMath_Mat4Index( 2u, 2u )] = c;
    
    return result;
}

mat4_t CypherMath_Mat4RotateZ( const common::f32 radians )
{
    mat4_t result = CypherMath_Mat4Identity();
    
    const common::f32 c = std::cos( radians );
    const common::f32 s = std::sin( radians );
    
    result.m[CypherMath_Mat4Index( 0u, 0u )] = c;
    result.m[CypherMath_Mat4Index( 0u, 1u )] = s;
    result.m[CypherMath_Mat4Index( 1u, 0u )] = -s;
    result.m[CypherMath_Mat4Index( 1u, 1u )] = c;
    
    return result;
}

mat4_t CypherMath_Mat4RotateAxis( const vec3_t &axis, const common::f32 radians )
{
    const vec3_t n = CypherMath_Vec3Normalize( axis );
    
    if ( CypherMath_Vec3LengthSquared( n ) <= MATH_EPSILON_F ) {
        return CypherMath_Mat4Identity();
    }
    
    const common::f32 c = std::cos( radians );     //cos(theta)
    const common::f32 s = std::sin( radians );     //sin(theta)
    const common::f32 one_minus_c = 1.0f - c;      //1 minus cos(theta)
    
    const common::f32 x = n.x;
    const common::f32 y = n.y;
    const common::f32 z = n.z;
    
    mat4_t result = CypherMath_Mat4Identity();
    
    /*
     * Rodirigues formula for rotation around arbitrary axis
     * displayed in matrix form.
     * 
     * Rotation around some vector axis, we normalize it always so we dont have
     * some weird scaling issues later. Also radians by how much we rotate around that
     * axis.
     */
    
    result.m[CypherMath_Mat4Index( 0u, 0u )] = c + x * x * one_minus_c;
    result.m[CypherMath_Mat4Index( 0u, 1u )] = y * x * one_minus_c + z * s;
    result.m[CypherMath_Mat4Index( 0u, 2u )] = z * x * one_minus_c - y * s;

    result.m[CypherMath_Mat4Index( 1u, 0u )] = x * y * one_minus_c - z * s;
    result.m[CypherMath_Mat4Index( 1u, 1u )] = c + y * y * one_minus_c;
    result.m[CypherMath_Mat4Index( 1u, 2u )] = z * y * one_minus_c + x * s;

    result.m[CypherMath_Mat4Index( 2u, 0u )] = x * z * one_minus_c + y * s;
    result.m[CypherMath_Mat4Index( 2u, 1u )] = y * z * one_minus_c - x * s;
    result.m[CypherMath_Mat4Index( 2u, 2u )] = c + z * z * one_minus_c;
    
    return result;
}

mat4_t CypherMath_Mat4Transpose( const mat4_t &m ) 
{
    mat4_t result = CypherMath_Mat4Zero();
    
    /*
     * Just pure swapping of rows and columns
     * [a b c d ]
     * [e f g h ]
     * [i j k l ]
     * [m n o p ]
     * ->
     * [a e i m ]
     * [b f j n ]
 .   * [c ....]
     * and so forth.
     */   
    for ( common::u32 column = 0; column < 4u; ++column ) {
        for ( common::u32 row = 0; row < 4u; ++row ) {
            result.m[CypherMath_Mat4Index( column, row )] = m.m[CypherMath_Mat4Index( row, column )];
        }
    }
    
    return result;
}

vec4_t CypherMath_Mat4MulVec4( const mat4_t &m, const vec4_t &v ) 
{
    return vec4_t{
        m.m[CypherMath_Mat4Index( 0u, 0u )] * v.x +
        m.m[CypherMath_Mat4Index( 1u, 0u )] * v.y +
        m.m[CypherMath_Mat4Index( 2u, 0u )] * v.z +
        m.m[CypherMath_Mat4Index( 3u, 0u )] * v.w,
        
        m.m[CypherMath_Mat4Index( 0u, 1u )] * v.x +
        m.m[CypherMath_Mat4Index( 1u, 1u )] * v.y +
        m.m[CypherMath_Mat4Index( 2u, 1u )] * v.z +
        m.m[CypherMath_Mat4Index( 3u, 1u )] * v.w,
        
        m.m[CypherMath_Mat4Index( 0u, 2u )] * v.x +
        m.m[CypherMath_Mat4Index( 1u, 2u )] * v.y +
        m.m[CypherMath_Mat4Index( 2u, 2u )] * v.z +
        m.m[CypherMath_Mat4Index( 3u, 2u )] * v.w,
        
        m.m[CypherMath_Mat4Index( 0u, 3u )] * v.x +
        m.m[CypherMath_Mat4Index( 1u, 3u )] * v.y +
        m.m[CypherMath_Mat4Index( 2u, 3u )] * v.z +
        m.m[CypherMath_Mat4Index( 3u, 3u )] * v.w,
    };
}

/*
 * Here the w in vector4 is 1!
 */
vec3_t CypherMath_Mat4TransformPoint( const mat4_t &m, const vec3_t &v )
{
    
    /*
     * If the w = 1 we determine this as the point.
     * According to those rules we decide what to do.
     * 
     * So for a point we transform it like a regular point, moving it by some
     * points,
     * 
     * A direction is not being moved, simply because a direction is an arrow
     * a vector that we only care about where it is pointing to, we dont really care 
     * about its length or anything, its numerical values, other then where it points 
     * to.
     * 
     */
    const vec4_t result = CypherMath_Mat4MulVec4( m, vec4_t{ v.x, v.y, v.z, 1.0f } );
    
    if ( result.w != 0.0f && result.w != 1.0f ) {
        return vec3_t{
            result.x / result.w,
            result.y / result.w,
            result.z / result.w
        };
    }
    
    return vec3_t{ result.x, result.y, result.z };
}
/*
 * Here the w in vector4 is 0!
 */
vec3_t CypherMath_Mat4TransformDirection( const mat4_t &m, const vec3_t &v ) 
{
    const vec4_t result = CypherMath_Mat4MulVec4( m, vec4_t{ v.x, v.y, v.z, 0.0f } );
    
    return vec3_t{ result.x, result.y, result.z };    
}

/*
================
CypherMath_Mat4Perspective

Builds an OpenGL-style right-handed perspective projection matrix.
Camera looks down -Z.
================
*/
mat4_t CypherMath_Mat4Perspective( common::f32 fov_y_radians, common::f32 aspect_ratio, common::f32 near_z, common::f32 far_z )
{
    mat4_t result = CypherMath_Mat4Zero();
    
    if ( aspect_ratio <= MATH_EPSILON_F || near_z <= MATH_EPSILON_F || far_z <= near_z ) {
        return result;
    }
    
    /*
     * OpenGL like formula for the projection matrix perspective.
     */
    
    const common::f32 f = 1.0f / std::tan( fov_y_radians * 0.5f );
    
    result.m[CypherMath_Mat4Index( 0u, 0u )] = f / aspect_ratio;
    result.m[CypherMath_Mat4Index( 1u, 1u )] = f;
    result.m[CypherMath_Mat4Index( 2u, 2u )] = ( far_z + near_z ) / ( near_z - far_z );
    result.m[CypherMath_Mat4Index( 2u, 3u )] = -1.0f; 
    result.m[CypherMath_Mat4Index( 3u, 2u )] = ( 2.0f * far_z * near_z ) / ( near_z - far_z ); 
    
    return result;
}

mat4_t CypherMath_Mat4Ortho( const common::f32 left, const common::f32 right, const common::f32 bottom, const common::f32 top, const common::f32 near_z, const common::f32 far_z )
{
    mat4_t result = CypherMath_Mat4Zero();
    
    const common::f32 width = right - left;
    const common::f32 height = top - bottom;
    const common::f32 depth = far_z - near_z;
    
    if ( std::fabs( width ) <= MATH_EPSILON_F ||
         std::fabs( height ) <= MATH_EPSILON_F ||
         std::fabs( depth ) <= MATH_EPSILON_F ) {
        return result;
    } 
    
    result.m[CypherMath_Mat4Index( 0u, 0u )] = 2.0f / width;
    result.m[CypherMath_Mat4Index( 1u, 1u )] = 2.0f / height;
    result.m[CypherMath_Mat4Index( 2u, 2u )] = -2.0f / depth;
    result.m[CypherMath_Mat4Index( 3u, 0u )] = -( right + left ) / width;
    result.m[CypherMath_Mat4Index( 3u, 1u )] = -( top + bottom ) / height;
    result.m[CypherMath_Mat4Index( 3u, 2u )] = -( far_z + near_z ) / depth;
    result.m[CypherMath_Mat4Index( 3u, 3u )] = 1.0f;
    
    return result;
}
/*
================
CypherMath_Mat4LookAt

Builds a right-handed OpenGL-style view matrix.
eye is the camera position, target is what it looks at, up keeps the camera upright.
================
*/
mat4_t CypherMath_Mat4LookAt( const vec3_t &eye, const vec3_t &target, const vec3_t &up ) 
{
    mat4_t result = CypherMath_Mat4Identity();
    const vec3_t forward = CypherMath_Vec3Normalize( CypherMath_Vec3Sub( target, eye ) );
    if ( CypherMath_Vec3LengthSquared( forward ) <= MATH_EPSILON_F ) {
        return CypherMath_Mat4Identity();
    }
    
    const vec3_t right = CypherMath_Vec3Normalize( CypherMath_Vec3Cross( forward, up ) );
    if ( CypherMath_Vec3LengthSquared( right ) <= MATH_EPSILON_F ) {
        return CypherMath_Mat4Identity();
    } 
    
    // @NOTE: It is already normalized since these other two are for sure up to this point.
    const vec3_t camera_up = CypherMath_Vec3Cross( right, forward );
    
    result.m[CypherMath_Mat4Index( 0u, 0u )] = right.x;
    result.m[CypherMath_Mat4Index( 0u, 1u )] = camera_up.x;
    result.m[CypherMath_Mat4Index( 0u, 2u )] = -forward.x;

    result.m[CypherMath_Mat4Index( 1u, 0u )] = right.y;
    result.m[CypherMath_Mat4Index( 1u, 1u )] = camera_up.y;
    result.m[CypherMath_Mat4Index( 1u, 2u )] = -forward.y;

    result.m[CypherMath_Mat4Index( 2u, 0u )] = right.z;
    result.m[CypherMath_Mat4Index( 2u, 1u )] = camera_up.z;
    result.m[CypherMath_Mat4Index( 2u, 2u )] = -forward.z;

    result.m[CypherMath_Mat4Index( 3u, 0u )] = -CypherMath_Vec3Dot( right, eye );
    result.m[CypherMath_Mat4Index( 3u, 1u )] = -CypherMath_Vec3Dot( camera_up, eye );
    result.m[CypherMath_Mat4Index( 3u, 2u )] = CypherMath_Vec3Dot( forward, eye );
    
    return result;
}

/*
================
CypherMath_Mat4TranslationRotationScale

Builds a model matrix for each and every single object that will be created in the game.
Each model matrix is unique to each and every single object.
================
*/
mat4_t CypherMath_Mat4TranslationRotationScale( const vec3_t &position, const quat_t &orientation, const vec3_t &scale )
{
    mat4_t result = CypherMath_Mat4Identity();
    const mat4_t translation_matrix = CypherMath_Mat4Translation( position );
    const mat4_t rotation_matrix    = CypherMath_QuatToMat4( orientation );
    const mat4_t scale_matrix       = CypherMath_Mat4Scale( scale );
    
    result = CypherMath_Mat4Multiply( translation_matrix, CypherMath_Mat4Multiply( rotation_matrix, scale_matrix ) );
    return result;
}

}          // namespace cypher::engine::math

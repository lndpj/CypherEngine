/*======================================================================
   File: math_plane.cpp
   Project: rengine
   Author: ksiric <email@example.com>
   Created: 2026-05-25 02:41:48
   Last Modified by: ksiric
   Last Modified: 2026-06-02 20:51:35
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
    
plane_t Math_PlaneFromPointNormal( const vec3_t &point, const vec3_t &normal )
{
    /*
     * Forming a plane out of normal vector and a point vector somewhere in space.
     */
    
    plane_t result{};
    
    const vec3_t normalized_normal = Math_Vec3Normalize( normal );
    
    if ( Math_Vec3LengthSquared( normalized_normal ) <= MATH_EPSILON_F ) {
        return result;
    }   
    
    result = { normalized_normal, Math_Vec3Dot( normalized_normal, point ) };
    return result;
}

plane_t Math_PlaneFromPoints( const vec3_t &p0, const vec3_t &p1, const vec3_t &p2 )
{
    /*
     * Forming a plane out of triangles essentially, so three vectors( vertices ).
     * Might be useful later for creating brushes, brush cpollisions, BSP faces, traingle planes etc.
     */
    
    plane_t result{};
    
    const vec3_t edge1 = Math_Vec3Sub( p1, p0 );
    const vec3_t edge2 = Math_Vec3Sub( p2, p0 );
    const vec3_t normal = Math_Vec3Normalize( Math_Vec3Cross( edge1, edge2 ) );
    
    if ( Math_Vec3LengthSquared( normal ) <= MATH_EPSILON_F ) {
        return plane_t{};
    }
    
    result = { normal, Math_Vec3Dot( normal, p0 ) };
    
    return result;
}


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

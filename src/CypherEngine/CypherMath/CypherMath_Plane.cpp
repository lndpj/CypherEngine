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
#include "CypherEngine/CypherMath/CypherMath_Plane.h"
#include "CypherEngine/CypherMath/CypherMath_Vec.h"

namespace cypher::engine::math
{

plane_t CypherMath_PlaneFromPointNormal( const vec3_t &point, const vec3_t &normal )
{
    /*
     * Forming a plane out of normal vector and a point vector somewhere in space.
     */

    plane_t result{};

    const vec3_t normalizedNormal = CypherMath_Vec3Normalize( normal );

    if ( CypherMath_Vec3LengthSquared( normalizedNormal ) <= MATH_EPSILON_F ) {
        return result;
    }

    result = { normalizedNormal, CypherMath_Vec3Dot( normalizedNormal, point ) };
    return result;
}

plane_t CypherMath_PlaneFromPoints( const vec3_t &p0, const vec3_t &p1, const vec3_t &p2 )
{
    /*
     * Forming a plane out of triangles essentially, so three vectors( vertices ).
     * Might be useful later for creating brushes, brush cpollisions, BSP faces, traingle planes etc.
     */

    plane_t result{};

    const vec3_t edge1 = CypherMath_Vec3Sub( p1, p0 );
    const vec3_t edge2 = CypherMath_Vec3Sub( p2, p0 );
    const vec3_t normal = CypherMath_Vec3Normalize( CypherMath_Vec3Cross( edge1, edge2 ) );

    if ( CypherMath_Vec3LengthSquared( normal ) <= MATH_EPSILON_F ) {
        return plane_t{};
    }

    result = { normal, CypherMath_Vec3Dot( normal, p0 ) };

    return result;
}


common::f32 CypherMath_PlaneDistance( const plane_t &plane, const vec3_t &v )
{
    /*
     * Corresponds to either giving a positive or negative value.
     * Positive value corresponds to a point being in front of the plane.
     * Negative value corresponds to a point being behind the plane.
     */
    return CypherMath_Vec3Dot( plane.normal, v ) - plane.dist;
}

bool CypherMath_PlanePointFront( const plane_t &plane, const vec3_t &v )
{
    /*
    const common::f32 vec_dot = CypherMath_Vec3Dot( plane.normal , v );
    if ( ( vec_dot - plane.dist ) < 0.0f ) {
        return false;
    }
    return true;
    */
    return CypherMath_PlaneDistance( plane, v ) > MATH_EPSILON_F;
}

bool CypherMath_PlanePointBack( const plane_t &plane, const vec3_t &v )
{
    /*
    const common::f32 vec_dot = CypherMath_Vec3Dot( plane.normal , v );
    if ( ( vec_dot - plane.dist ) > 0.0f ) {
        return false;
    }
    return true;
    */
    return CypherMath_PlaneDistance( plane, v ) < -MATH_EPSILON_F;
}

bool CypherMath_PlanePointOn( const plane_t &plane, const vec3_t &v, common::f32 epsilon )
{
    const common::f32 distance = CypherMath_PlaneDistance( plane, v );

    return distance >= -epsilon && distance <= epsilon;
}

}       // namespace cypher::engine::math

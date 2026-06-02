/*======================================================================
   File: math_frustum.cpp
   Project: rengine
   Author: ksiric <email@example.com>
   Created: 2026-05-26 19:51:53
   Last Modified by: ksiric
   Last Modified: 2026-06-02 21:53:33
   ---------------------------------------------------------------------
   Description:
       
   ---------------------------------------------------------------------
   License: 
   Company: 
   Version: 0.1.0
 ======================================================================
                                                                       */
#include "rengine/math/math_bounds.h"
#include "rengine/math/math_plane.h"
#include "rengine/math/math_types.h"
#include "rengine/math/math_frustum.h"
#include "rengine/math/math_mat.h"
#include "rengine/math/math_vec.h"

#include <cmath>            // sqrt

namespace reap::rengine::math
{

static plane_t Math_FrustumMakePlane( const rcommon::f32 a, const rcommon::f32 b, const rcommon::f32 c, const rcommon::f32 d ) 
{
    // a, b and c are part of the normal vector, d is just the offset, position of the plane.
    const rcommon::f32 length = std::sqrt( a * a + b * b + c * c );
    
    if ( length <= MATH_EPSILON_F ) {
        return plane_t{};
    }
    
    // for normalization process
    const rcommon::f32 inv_length = 1.0f / length;
    // negative value is used because of the way the engine is written itself.
    plane_t result = { vec3_t { a * inv_length, b * inv_length, c * inv_length }, -d * inv_length };
    
    return result;
}

frustum_t Math_FrustumFromProjectionView( const mat4_t &projection_view )
{
    frustum_t result{};
    // GRIBB-HARTMANM METHOD for extrating all the planes effectively out of the P-V matrix.   
    
    result.planes[FRUSTUM_PLANE_LEFT] = Math_FrustumMakePlane(
        Math_Mat4Get( projection_view, 0u, 3u ) + Math_Mat4Get( projection_view, 0u, 0u ),
        Math_Mat4Get( projection_view, 1u, 3u ) + Math_Mat4Get( projection_view, 1u, 0u ),
        Math_Mat4Get( projection_view, 2u, 3u ) + Math_Mat4Get( projection_view, 2u, 0u ),
        Math_Mat4Get( projection_view, 3u, 3u ) + Math_Mat4Get( projection_view, 3u, 0u )
    );

    result.planes[FRUSTUM_PLANE_RIGHT] = Math_FrustumMakePlane(
        Math_Mat4Get( projection_view, 0u, 3u ) - Math_Mat4Get( projection_view, 0u, 0u ),
        Math_Mat4Get( projection_view, 1u, 3u ) - Math_Mat4Get( projection_view, 1u, 0u ),
        Math_Mat4Get( projection_view, 2u, 3u ) - Math_Mat4Get( projection_view, 2u, 0u ),
        Math_Mat4Get( projection_view, 3u, 3u ) - Math_Mat4Get( projection_view, 3u, 0u )
    );

    result.planes[FRUSTUM_PLANE_BOTTOM] = Math_FrustumMakePlane(
        Math_Mat4Get( projection_view, 0u, 3u ) + Math_Mat4Get( projection_view, 0u, 1u ),
        Math_Mat4Get( projection_view, 1u, 3u ) + Math_Mat4Get( projection_view, 1u, 1u ),
        Math_Mat4Get( projection_view, 2u, 3u ) + Math_Mat4Get( projection_view, 2u, 1u ),
        Math_Mat4Get( projection_view, 3u, 3u ) + Math_Mat4Get( projection_view, 3u, 1u )
    );

    result.planes[FRUSTUM_PLANE_TOP] = Math_FrustumMakePlane(
        Math_Mat4Get( projection_view, 0u, 3u ) - Math_Mat4Get( projection_view, 0u, 1u ),
        Math_Mat4Get( projection_view, 1u, 3u ) - Math_Mat4Get( projection_view, 1u, 1u ),
        Math_Mat4Get( projection_view, 2u, 3u ) - Math_Mat4Get( projection_view, 2u, 1u ),
        Math_Mat4Get( projection_view, 3u, 3u ) - Math_Mat4Get( projection_view, 3u, 1u )
    );

    result.planes[FRUSTUM_PLANE_NEAR] = Math_FrustumMakePlane(
        Math_Mat4Get( projection_view, 0u, 3u ) + Math_Mat4Get( projection_view, 0u, 2u ),
        Math_Mat4Get( projection_view, 1u, 3u ) + Math_Mat4Get( projection_view, 1u, 2u ),
        Math_Mat4Get( projection_view, 2u, 3u ) + Math_Mat4Get( projection_view, 2u, 2u ),
        Math_Mat4Get( projection_view, 3u, 3u ) + Math_Mat4Get( projection_view, 3u, 2u )
    );

    result.planes[FRUSTUM_PLANE_FAR] = Math_FrustumMakePlane(
        Math_Mat4Get( projection_view, 0u, 3u ) - Math_Mat4Get( projection_view, 0u, 2u ),
        Math_Mat4Get( projection_view, 1u, 3u ) - Math_Mat4Get( projection_view, 1u, 2u ),
        Math_Mat4Get( projection_view, 2u, 3u ) - Math_Mat4Get( projection_view, 2u, 2u ),
        Math_Mat4Get( projection_view, 3u, 3u ) - Math_Mat4Get( projection_view, 3u, 2u )
    );    
    
    return result;
}

/*
 * ===============
 * HELPER FRUSTUM FUNCS
 * ================
 */
bool Math_FrustumContainsPoint( const frustum_t &frustum, const vec3_t &point )
{
    for( rcommon::u32 i = 0u; i < FRUSTUM_PLANE_COUNT; ++i ) {
        if ( Math_PlaneDistance( frustum.planes[i], point ) < -MATH_EPSILON_F ) {
            return false;
        }
    }
    return true;
}

bool Math_FrustumIntersectsBounds( const frustum_t &frustum, const bounds_t &bounds )
{
    // finding the center of the boundings
    const vec3_t center = Math_BoundsCenter( bounds );
    const vec3_t size = Math_BoundsSize( bounds );
    const vec3_t halfs = Math_Vec3Scale( size, 0.5f );
    
    for( rcommon::u32 i = 0; i < FRUSTUM_PLANE_COUNT; ++i ) {
        const plane_t &plane = frustum.planes[i];
        
        const rcommon::f32 distance = Math_PlaneDistance( plane, center );
        
        const rcommon::f32 radius = halfs.x * std::fabs( plane.normal.x ) + halfs.y * std::fabs( plane.normal.y ) + halfs.z * std::fabs( plane.normal.z );
        
        if ( distance < -radius ) {
            return false;
        }        
    } 
    return true;
}
    
}       // namespace reap::rengine::math

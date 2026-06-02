/*======================================================================
   File: math_frustum.cpp
   Project: rengine
   Author: ksiric <email@example.com>
   Created: 2026-05-26 19:51:53
   Last Modified by: ksiric
   Last Modified: 2026-05-31 13:31:19
   ---------------------------------------------------------------------
   Description:
       
   ---------------------------------------------------------------------
   License: 
   Company: 
   Version: 0.1.0
 ======================================================================
                                                                       */
#include "rengine/math/math_types.h"
#include "rengine/math/math_frustum.h"
#include "rengine/math/math_mat.h"

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
    
    result.planes[FRUSTUM_PLANE_LEFT] = Math_FrustumMakePlane(
        Math_Mat4Get( projection_view, 0u,  3u ) + Math_Mat4Get( projection_view, 0u, 0u ),
        Math_Mat4Get( projection_view, )
         );
    
}





    
}       // namespace reap::rengine::math

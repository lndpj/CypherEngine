/*======================================================================
   File: math_ray.cpp
   Project: rengine
   Author: ksiric <email@example.com>
   Created: 2026-05-26 19:06:04
   Last Modified by: ksiric
   Last Modified: 2026-05-26 20:08:11
   ---------------------------------------------------------------------
   Description:
       
   ---------------------------------------------------------------------
   License: 
   Company: 
   Version: 0.1.0
 ======================================================================
                                                                       */

#include "rengine/math/math_ray.h"
#include "rengine/math/math_vec.h"

#include <cmath>

namespace reap::rengine::math
{

vec3_t Math_RayPointAt( const ray_t &ray, rcommon::f32 t_units )
{
    return Math_Vec3Add( ray.origin, Math_Vec3Scale( ray.direction, t_units ) );
}

bool Math_RayIntersectsPlane( const ray_t &ray, const plane_t &plane, rcommon::f32 &out_t_units )
{
    rcommon::f32 denom = Math_Vec3Dot( plane.normal, ray.direction );
    if ( std::fabs( denom ) <= MATH_EPSILON_F ) {
        return false;
    }
    out_t_units = ( plane.dist - Math_Vec3Dot( plane.normal, ray.origin ) ) / denom;
    return out_t_units >= 0.0f;
}
bool Math_RayIntersectsBounds( const ray_t &ray, const bounds_t &bounds, rcommon::f32 &out_tmin, rcommon::f32 &out_tmax )
{
    
    /*
     * Using a slab metho for doing this calculation.
     * 
     * AABB slab method, where we find the intersection of each and every single of 
     * the X, Y and Z slabs.
     * 
     * Each of those slabs has a min and max value, and we try to find the entry 
     * start 
     */

    rcommon::f32 t_units_min = 0.0f;
    rcommon::f32 t_units_max = 3.402823466e+38f;
    
    rcommon::f32 origin[3]      = { ray.origin.x, ray.origin.y, ray.origin.z };
    rcommon::f32 direction[3]   = { ray.direction.x, ray.direction.y, ray.direction.z };
    rcommon::f32 mins[3]        = { bounds.mins.x, bounds.mins.y, bounds.mins.z };
    rcommon::f32 maxs[3]        = { bounds.maxs.x, bounds.maxs.y, bounds.maxs.z };
    
    for ( rcommon::u32 i = 0; i < 3u; ++i ) {
        if ( std::fabs( direction[i] ) <= MATH_EPSILON_F ) {
            if ( origin[i] < mins[i] || origin[i] > maxs[i] ) {
                return false;
            }
            continue;
        }
        // @NOTE: Same as dividing, but this is cleaner somehow.
        rcommon::f32 inv_direction = 1.0f / direction[i];
        rcommon::f32 t1 = ( mins[i] - origin[i] ) * inv_direction;
        rcommon::f32 t2 = ( maxs[i] - origin[i] ) * inv_direction;
        
        // If the ray is going in the opposite direction, so negative dir of the vector.
        if ( t1 > t2 ) {
            const rcommon::f32 temp = t1;
            t1 = t2;
            t2 = temp;
        }
        
        if ( t1 > t_units_min ) {
            t_units_min = t1;
        }
        
        if ( t2 < t_units_max ) {
            t_units_max = t2;
        }
        /*
         * It happened if and only if the tmin < tmax essentially.
         * Because if tmin is larger than tmax then the ray exited one slab window 
         * before it entered another.
         * 
         * And the ray needs to be inside all three of those at the same time 
         * to be intersecting.
         * 
         * [2, 5] -> X SLAB -> t1 = 2, t2 = 5, tmin = 2, tmax = 5
         * [3, 4] -> Y SLAB -> t1 = 3, t2 = 4, tmin = 2, tmax = 5
         * [5, 9] -> Z SLAB
         * 
         * Here not intersecting, it is -> t
         */
        if ( t_units_min > t_units_max ) {
            return false;
        }
    }
    
    out_tmin = t_units_min;
    out_tmax = t_units_max;
    
    return true;
}

}       // namespace reap::rengine::math

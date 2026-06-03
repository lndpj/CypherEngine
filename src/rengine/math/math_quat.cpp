/*======================================================================
   File: math_quat.cpp
   Project: rengine
   Author: ksiric <email@example.com>
   Created: 2026-05-24 14:46:51
   Last Modified by: ksiric
   Last Modified: 2026-06-03 10:06:54
   ---------------------------------------------------------------------
   Description:

   ---------------------------------------------------------------------
   License:
   Company:
   Version: 0.1.0
 ======================================================================
																	   */
#include "rengine/math/math_quat.h"
#include "rengine/math/math_mat.h"
#include "rengine/math/math_vec.h"

#include <cmath>

namespace reap::rengine::math {

quat_t Math_QuatIdentity() {
	return quat_t{ 0.0f, 0.0f, 0.0f, 1.0f };
}

quat_t Math_QuatMake( rcommon::f32 x, rcommon::f32 y, rcommon::f32 z, rcommon::f32 w ) {
	return quat_t{ x, y, z, w };
}

rcommon::f32 Math_QuatDot( const quat_t &q1, const quat_t &q2 ) {
	return q1.x * q2.x + q1.y * q2.y + q1.z * q2.z + q1.w * q2.w;
}

quat_t Math_QuatNormalize( const quat_t &q ) {
	const rcommon::f32 quat_squared = Math_QuatDot( q, q );

	if ( quat_squared <= MATH_EPSILON_F ) {
		return Math_QuatIdentity();
	}

	const rcommon::f32 inv_quat_squared = 1.0f / std::sqrt( quat_squared );
	return quat_t{
		q.x * inv_quat_squared,
		q.y * inv_quat_squared,
		q.z * inv_quat_squared,
		q.w * inv_quat_squared
	};
}

quat_t Math_QuatConjugate( const quat_t &q ) {
	return quat_t{
		( -q.x ),
		( -q.y ),
		( -q.z ),
		( q.w )
	};
}

quat_t Math_QuatInverse( const quat_t &q ) {
	const rcommon::f32 quat_squared = Math_QuatDot( q, q );

	if ( quat_squared <= MATH_EPSILON_F ) {
		return Math_QuatIdentity();
	}

	const quat_t quat_conjugate = Math_QuatConjugate( q );
	const rcommon::f32 quat_inv_squared = 1.0f / quat_squared;
	return quat_t{
		quat_conjugate.x * quat_inv_squared,
		quat_conjugate.y * quat_inv_squared,
		quat_conjugate.z * quat_inv_squared,
		quat_conjugate.w * quat_inv_squared,
	};
}

quat_t Math_QuatMultiply( const quat_t &q1, const quat_t &q2 ) {
	return quat_t{
		q1.w * q2.x + q1.x * q2.w + q1.y * q2.z - q1.z * q2.y,
		q1.w * q2.y - q1.x * q2.z + q1.y * q2.w + q1.z * q2.x,
		q1.w * q2.z + q1.x * q2.y - q1.y * q2.x + q1.z * q2.w,
		q1.w * q2.w - q1.x * q2.x - q1.y * q2.y - q1.z * q2.z
	};
}

quat_t Math_QuatFromAxis( const vec3_t &axis, const rcommon::f32 radians ) {
	const vec3_t normalized_axis = Math_Vec3Normalize( axis );

	if ( Math_Vec3LengthSquared( normalized_axis ) <= MATH_EPSILON_F ) {
		return Math_QuatIdentity();
	}

	const rcommon::f32 half_angle = radians * 0.5f;
	const rcommon::f32 s = std::sin( half_angle );
	const rcommon::f32 c = std::cos( half_angle );

	return Math_QuatNormalize( quat_t{
		normalized_axis.x * s,
		normalized_axis.y * s,
		normalized_axis.z * s,
		c } );
}

quat_t Math_QuatFromEuler( const rcommon::f32 pitch, const rcommon::f32 yaw, const rcommon::f32 roll ) {
	const quat_t quat_pitch = Math_QuatFromAxis( vec3_t{ 1.0f, 0.0f, 0.0f }, pitch );
	const quat_t quat_yaw = Math_QuatFromAxis( vec3_t{ 0.0f, 1.0f, 0.0f }, yaw );
	const quat_t quat_roll = Math_QuatFromAxis( vec3_t{ 0.0f, 0.0f, 1.0f }, roll );
    
    /*
     * For this game engine and this game that the engine will be used
     * The convention I decided is:
     * yaw * pitch * roll!
     * yaw      -> left and right rotation
     * pitch    -> up and down rotation
     * roll     -> tilt rotation 
     * Because we are making a FPS arena wave shooter, this would be the best order
     * of things.
     */
	return Math_QuatNormalize( Math_QuatMultiply( Math_QuatMultiply( quat_yaw, quat_pitch ), quat_roll ) );
}

vec3_t Math_QuatForwardVec3( const quat_t &q )
{
    vec3_t result = {};
    result = Math_QuatRotateVec3( q, vec3_t{ 0.0f, 0.0f, -1.0f } );
    return result;
}

vec3_t Math_QuatRightVec3( const quat_t &q )
{
    vec3_t result = {};
    result = Math_QuatRotateVec3( q, vec3_t{ 1.0f, 0.0f, 0.0f } );
    return result;
}

vec3_t Math_QuatUpVec3( const quat_t &q )
{
    vec3_t result = {};
    result = Math_QuatRotateVec3( q, vec3_t{ 0.0f, 1.0f, 0.0f } );
    return result;
}

mat4_t Math_QuatToMat4( const quat_t &q ) {
	mat4_t result = Math_Mat4Identity();
	const quat_t quat_normalize = Math_QuatNormalize( q );

    const rcommon::f32 x = quat_normalize.x;
    const rcommon::f32 y = quat_normalize.y;
    const rcommon::f32 z = quat_normalize.z;
    const rcommon::f32 w = quat_normalize.w;
    
    result.m[Math_Mat4Index( 0u, 0u )] = 1.0f - 2.0f * y * y - 2.0f * z * z;
    result.m[Math_Mat4Index( 0u, 1u )] = 2.0f * x * y + 2.0f * w * z;
    result.m[Math_Mat4Index( 0u, 2u )] = 2.0f * x * z - 2.0f * w * y;

    result.m[Math_Mat4Index( 1u, 0u )] = 2.0f * x * y - 2.0f * w * z;
    result.m[Math_Mat4Index( 1u, 1u )] = 1.0f - 2.0f * x * x - 2.0f * z * z;
    result.m[Math_Mat4Index( 1u, 2u )] = 2.0f * y * z + 2.0f * w * x;

    result.m[Math_Mat4Index( 2u, 0u )] = 2.0f * x * z + 2.0f * w * y;
    result.m[Math_Mat4Index( 2u, 1u )] = 2.0f * y * z - 2.0f * w * x;
    result.m[Math_Mat4Index( 2u, 2u )] = 1.0f - 2.0f * x * x - 2.0f * y * y;
    
	return result;
}

vec3_t Math_QuatRotateVec3( const quat_t &q, const vec3_t &v ) {
	/*
	 * For a normalized quaternion it is simply that the inverse quaternion is
	 * equal to the conjugate quaternion so that is all fine.
	 */
	const quat_t quat_normalize = Math_QuatNormalize( q );
	const quat_t quat_vector = { v.x, v.y, v.z, 0.0f };

	const quat_t rotated = Math_QuatMultiply( Math_QuatMultiply( quat_normalize, quat_vector ), Math_QuatConjugate( quat_normalize ) );

	return vec3_t{ rotated.x, rotated.y, rotated.z };
}

quat_t Math_QuatNLerp( const quat_t &q1, const quat_t &q2, const rcommon::f32 t ) {
	/*
	 * Using names such as start and end, makes it easier to orient on
	 * what is happening when doing the interpolation, regardless if it is
	 * the standard or the normalized quat interpolation.
	 *
	 * @NOTE: Same applied for the QuatSlerp!
	 */
	quat_t start = q1;
	quat_t end = q2;
	if ( Math_QuatDot( start, end ) < 0.0f ) {
		end = quat_t{ -end.x, -end.y, -end.z, -end.w };
	}

	return Math_QuatNormalize( quat_t{
		start.x + ( end.x - start.x ) * t,
		start.y + ( end.y - start.y ) * t,
		start.z + ( end.z - start.z ) * t,
		start.w + ( end.w - start.w ) * t,
	} );
}

quat_t Math_QuatSlerp( const quat_t &q1, const quat_t &q2, const rcommon::f32 t ) {
	quat_t start = Math_QuatNormalize( q1 );
	quat_t end = Math_QuatNormalize( q2 );
	rcommon::f32 quat_dot = Math_QuatDot( start, end );
	if ( quat_dot < 0.0f ) {
		end = quat_t{ -end.x, -end.y, -end.z, -end.w };
		quat_dot = -quat_dot;
	}

	if ( quat_dot > 0.995f ) {
		return Math_QuatNLerp( start, end, t );
	}

	if ( quat_dot > 1.0f ) {
		quat_dot = 1.0f;
	}

	const rcommon::f32 theta_0 = std::acos( quat_dot );
	const rcommon::f32 theta = theta_0 * t;

	const rcommon::f32 sin_theta = std::sin( theta );
	const rcommon::f32 sin_theta_0 = std::sin( theta_0 );

	const rcommon::f32 s0 = std::cos( theta ) - quat_dot * sin_theta / sin_theta_0;
	const rcommon::f32 s1 = sin_theta / sin_theta_0;

	return Math_QuatNormalize( quat_t{
		start.x * s0 + end.x * s1,
		start.y * s0 + end.y * s1,
		start.z * s0 + end.z * s1,
		start.w * s0 + end.w * s1 } );
}

} // namespace reap::rengine::math

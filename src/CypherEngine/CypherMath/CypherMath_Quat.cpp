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
#include "CypherEngine/CypherMath/CypherMath_Quat.h"
#include "CypherEngine/CypherMath/CypherMath_Mat.h"
#include "CypherEngine/CypherMath/CypherMath_Vec.h"

#include <cmath>

namespace cypher::engine::math {

quat_t CypherMath_QuatIdentity() {
	return quat_t{ 0.0f, 0.0f, 0.0f, 1.0f };
}

quat_t CypherMath_QuatMake( common::f32 x, common::f32 y, common::f32 z, common::f32 w ) {
	return quat_t{ x, y, z, w };
}

common::f32 CypherMath_QuatDot( const quat_t &q1, const quat_t &q2 ) {
	return q1.x * q2.x + q1.y * q2.y + q1.z * q2.z + q1.w * q2.w;
}

quat_t CypherMath_QuatNormalize( const quat_t &q ) {
	const common::f32 quat_squared = CypherMath_QuatDot( q, q );

	if ( quat_squared <= MATH_EPSILON_F ) {
		return CypherMath_QuatIdentity();
	}

	const common::f32 inv_quat_squared = 1.0f / std::sqrt( quat_squared );
	return quat_t{
		q.x * inv_quat_squared,
		q.y * inv_quat_squared,
		q.z * inv_quat_squared,
		q.w * inv_quat_squared
	};
}

quat_t CypherMath_QuatConjugate( const quat_t &q ) {
	return quat_t{
		( -q.x ),
		( -q.y ),
		( -q.z ),
		( q.w )
	};
}

quat_t CypherMath_QuatInverse( const quat_t &q ) {
	const common::f32 quat_squared = CypherMath_QuatDot( q, q );

	if ( quat_squared <= MATH_EPSILON_F ) {
		return CypherMath_QuatIdentity();
	}

	const quat_t quat_conjugate = CypherMath_QuatConjugate( q );
	const common::f32 quat_inv_squared = 1.0f / quat_squared;
	return quat_t{
		quat_conjugate.x * quat_inv_squared,
		quat_conjugate.y * quat_inv_squared,
		quat_conjugate.z * quat_inv_squared,
		quat_conjugate.w * quat_inv_squared,
	};
}

quat_t CypherMath_QuatMultiply( const quat_t &q1, const quat_t &q2 ) {
	return quat_t{
		q1.w * q2.x + q1.x * q2.w + q1.y * q2.z - q1.z * q2.y,
		q1.w * q2.y - q1.x * q2.z + q1.y * q2.w + q1.z * q2.x,
		q1.w * q2.z + q1.x * q2.y - q1.y * q2.x + q1.z * q2.w,
		q1.w * q2.w - q1.x * q2.x - q1.y * q2.y - q1.z * q2.z
	};
}

quat_t CypherMath_QuatFromAxis( const vec3_t &axis, const common::f32 radians ) {
	const vec3_t normalized_axis = CypherMath_Vec3Normalize( axis );

	if ( CypherMath_Vec3LengthSquared( normalized_axis ) <= MATH_EPSILON_F ) {
		return CypherMath_QuatIdentity();
	}

	const common::f32 half_angle = radians * 0.5f;
	const common::f32 s = std::sin( half_angle );
	const common::f32 c = std::cos( half_angle );

	return CypherMath_QuatNormalize( quat_t{
		normalized_axis.x * s,
		normalized_axis.y * s,
		normalized_axis.z * s,
		c } );
}

quat_t CypherMath_QuatFromEuler( const common::f32 pitch, const common::f32 yaw, const common::f32 roll ) {
	const quat_t quat_pitch = CypherMath_QuatFromAxis( vec3_t{ 1.0f, 0.0f, 0.0f }, pitch );
	const quat_t quat_yaw = CypherMath_QuatFromAxis( vec3_t{ 0.0f, 1.0f, 0.0f }, yaw );
	const quat_t quat_roll = CypherMath_QuatFromAxis( vec3_t{ 0.0f, 0.0f, 1.0f }, roll );
    
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
	return CypherMath_QuatNormalize( CypherMath_QuatMultiply( CypherMath_QuatMultiply( quat_yaw, quat_pitch ), quat_roll ) );
}

vec3_t CypherMath_QuatForwardVec3( const quat_t &q )
{
    vec3_t result = {};
    result = CypherMath_QuatRotateVec3( q, vec3_t{ 0.0f, 0.0f, -1.0f } );
    return result;
}

vec3_t CypherMath_QuatRightVec3( const quat_t &q )
{
    vec3_t result = {};
    result = CypherMath_QuatRotateVec3( q, vec3_t{ 1.0f, 0.0f, 0.0f } );
    return result;
}

vec3_t CypherMath_QuatUpVec3( const quat_t &q )
{
    vec3_t result = {};
    result = CypherMath_QuatRotateVec3( q, vec3_t{ 0.0f, 1.0f, 0.0f } );
    return result;
}

mat4_t CypherMath_QuatToMat4( const quat_t &q ) {
	mat4_t result = CypherMath_Mat4Identity();
	const quat_t quat_normalize = CypherMath_QuatNormalize( q );

    const common::f32 x = quat_normalize.x;
    const common::f32 y = quat_normalize.y;
    const common::f32 z = quat_normalize.z;
    const common::f32 w = quat_normalize.w;
    
    result.m[CypherMath_Mat4Index( 0u, 0u )] = 1.0f - 2.0f * y * y - 2.0f * z * z;
    result.m[CypherMath_Mat4Index( 0u, 1u )] = 2.0f * x * y + 2.0f * w * z;
    result.m[CypherMath_Mat4Index( 0u, 2u )] = 2.0f * x * z - 2.0f * w * y;

    result.m[CypherMath_Mat4Index( 1u, 0u )] = 2.0f * x * y - 2.0f * w * z;
    result.m[CypherMath_Mat4Index( 1u, 1u )] = 1.0f - 2.0f * x * x - 2.0f * z * z;
    result.m[CypherMath_Mat4Index( 1u, 2u )] = 2.0f * y * z + 2.0f * w * x;

    result.m[CypherMath_Mat4Index( 2u, 0u )] = 2.0f * x * z + 2.0f * w * y;
    result.m[CypherMath_Mat4Index( 2u, 1u )] = 2.0f * y * z - 2.0f * w * x;
    result.m[CypherMath_Mat4Index( 2u, 2u )] = 1.0f - 2.0f * x * x - 2.0f * y * y;
    
	return result;
}

vec3_t CypherMath_QuatRotateVec3( const quat_t &q, const vec3_t &v ) {
	/*
	 * For a normalized quaternion it is simply that the inverse quaternion is
	 * equal to the conjugate quaternion so that is all fine.
	 */
	const quat_t quat_normalize = CypherMath_QuatNormalize( q );
	const quat_t quat_vector = { v.x, v.y, v.z, 0.0f };

	const quat_t rotated = CypherMath_QuatMultiply( CypherMath_QuatMultiply( quat_normalize, quat_vector ), CypherMath_QuatConjugate( quat_normalize ) );

	return vec3_t{ rotated.x, rotated.y, rotated.z };
}

quat_t CypherMath_QuatNLerp( const quat_t &q1, const quat_t &q2, const common::f32 t ) {
	/*
	 * Using names such as start and end, makes it easier to orient on
	 * what is happening when doing the interpolation, regardless if it is
	 * the standard or the normalized quat interpolation.
	 *
	 * @NOTE: Same applied for the QuatSlerp!
	 */
	quat_t start = q1;
	quat_t end = q2;
	if ( CypherMath_QuatDot( start, end ) < 0.0f ) {
		end = quat_t{ -end.x, -end.y, -end.z, -end.w };
	}

	return CypherMath_QuatNormalize( quat_t{
		start.x + ( end.x - start.x ) * t,
		start.y + ( end.y - start.y ) * t,
		start.z + ( end.z - start.z ) * t,
		start.w + ( end.w - start.w ) * t,
	} );
}

quat_t CypherMath_QuatSlerp( const quat_t &q1, const quat_t &q2, const common::f32 t ) {
	quat_t start = CypherMath_QuatNormalize( q1 );
	quat_t end = CypherMath_QuatNormalize( q2 );
	common::f32 quat_dot = CypherMath_QuatDot( start, end );
	if ( quat_dot < 0.0f ) {
		end = quat_t{ -end.x, -end.y, -end.z, -end.w };
		quat_dot = -quat_dot;
	}

	if ( quat_dot > 0.995f ) {
		return CypherMath_QuatNLerp( start, end, t );
	}

	if ( quat_dot > 1.0f ) {
		quat_dot = 1.0f;
	}

	const common::f32 theta_0 = std::acos( quat_dot );
	const common::f32 theta = theta_0 * t;

	const common::f32 sin_theta = std::sin( theta );
	const common::f32 sin_theta_0 = std::sin( theta_0 );

	const common::f32 s0 = std::cos( theta ) - quat_dot * sin_theta / sin_theta_0;
	const common::f32 s1 = sin_theta / sin_theta_0;

	return CypherMath_QuatNormalize( quat_t{
		start.x * s0 + end.x * s1,
		start.y * s0 + end.y * s1,
		start.z * s0 + end.z * s1,
		start.w * s0 + end.w * s1 } );
}

} // namespace cypher::engine::math

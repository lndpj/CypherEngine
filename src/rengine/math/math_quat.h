#pragma once

#include "rengine/math/math_types.h"

namespace reap::rengine::math
{

struct quat_t {
    rcommon::f32 x{};
    rcommon::f32 y{};
    rcommon::f32 z{};
    rcommon::f32 w{ 1.0f };
};

quat_t Math_QuatIdentity();

quat_t Math_QuatMake( rcommon::f32 x, rcommon::f32 y, rcommon::f32 z, rcommon::f32 w );

rcommon::f32 Math_QuatDot( const quat_t &q1, const quat_t &q2 );

quat_t Math_QuatNormalize( const quat_t &q );

quat_t Math_QuatConjugate( const quat_t &q );

quat_t Math_QuatInverse( const quat_t &q );

quat_t Math_QuatMultiply( const quat_t &q1, const quat_t &q2 );

quat_t Math_QuatFromAxis( const vec3_t &axis, const rcommon::f32 radians );

quat_t Math_QuatFromEuler( const rcommon::f32 pitch, const rcommon::f32 yaw, const rcommon::f32 roll );

mat4_t Math_QuatToMat4( const quat_t &q );

vec3_t Math_QuatRotateVec3( const quat_t &q, const vec3_t &v );

quat_t Math_QuatNLerp( const quat_t &q1, const quat_t &q2, const rcommon::f32 t );    

quat_t Math_QuatSlerp( const quat_t &q1, const quat_t &q2, const rcommon::f32 t );    
    
}       // namespace reap::rengine::math

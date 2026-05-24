#pragma once

#include "rengine/math/math_types.h"

namespace reap::rengine::math
{

/*
================
Matrix Convention

Column-major storage, column vectors, OpenGL-style transform order.

Element index:
    m[column * 4 + row]

Transform order:
    clip = projection * view * model * local_position

World convention:
    +X right
    +Y up
    -Z forward
================
*/
    
constexpr inline rcommon::u32 Math_Mat4Index( const rcommon::u32 column, const rcommon::u32 row ) {
    return column * 4u + row;
}

constexpr inline rcommon::f32 Math_Mat4Get( const mat4_t &m, const rcommon::u32 column, const rcommon::u32 row ) {
    return m.m[Math_Mat4Index( column, row )];
}

constexpr inline void Math_Mat4Set( mat4_t &m, const rcommon::u32 column, const rcommon::u32 row, const rcommon::f32 value ) {
    m.m[Math_Mat4Index( column, row )] = value;
}

mat4_t Math_Mat4Zero();

mat4_t Math_Mat4Identity();

mat4_t Math_Mat4Multiply( const mat4_t &m1, const mat4_t &m2 );

mat4_t Math_Mat4Translation( const vec3_t &translation_vector );

mat4_t Math_Mat4Scale( const vec3_t &scale );

mat4_t Math_Mat4RotateX( const rcommon::f32 radians );

mat4_t Math_Mat4RotateY( const rcommon::f32 radians );

mat4_t Math_Mat4RotateZ( const rcommon::f32 radians );

mat4_t Math_Mat4RotateAxis( const vec3_t &axis, const rcommon::f32 radians );

mat4_t Math_Mat4Transpose( const mat4_t &m );

vec4_t Math_Mat4MulVec4( const mat4_t &m, const vec4_t &v );

vec3_t Math_Mat4TransformPoint( const mat4_t &m, const vec3_t &v );

vec3_t Math_Mat4TransformDirection( const mat4_t &m, const vec3_t &v );

mat4_t Math_Mat4Perspective( rcommon::f32 fov_y_radians, rcommon::f32 aspect_ratio, rcommon::f32 near_z, rcommon::f32 far_z );

mat4_t Math_Mat4Ortho( const rcommon::f32 left, const rcommon::f32 right, const rcommon::f32 bottom, const rcommon::f32 top, const rcommon::f32 near_z, const rcommon::f32 far_z );



}       // namespace reap::rengine::math

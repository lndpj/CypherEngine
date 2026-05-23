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

constexpr inline void Math_Mat4Set( mat4_t &m, const rcommon::u32 column, const rcommon::u32 row, const rcommon::u32 value ) {
    m.m[Math_Mat4Index( column, row )] = value;
}

mat4_t Math_Mat4Zero();

mat4_t Math_Mat4Identity();




}       // namespace reap::rengine::math

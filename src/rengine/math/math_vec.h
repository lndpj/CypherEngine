#pragma once

#include "rengine/math/math_types.h"

namespace reap::rengine::math
{
    
/*
================
Vec2 Constructors
================
*/

constexpr inline vec2_t Math_Vec2() {
    return vec2_t{};
}

constexpr inline vec2_t Math_Vec2( const rcommon::f32 x, const rcommon::f32 y ) {
    return vec2_t{ x, y };
}

constexpr inline vec2_t Math_Vec2Zero() {
    return vec2_t{};
}

/*
================
Vec2 Operations
================
*/

constexpr inline vec2_t Math_Vec2Add( const vec2_t &v1, const vec2_t &v2 ) {
    return vec2_t{ v1.x + v2.x, v1.y + v2.y };
}

constexpr inline vec2_t Math_Vec2Sub( const vec2_t &v1, const vec2_t &v2 ) {
    return vec2_t{ v1.x - v2.x, v1.y - v2.y };
}

constexpr inline vec2_t Math_Vec2Scale( const vec2_t &v, const rcommon::f32 scale ) {
    return vec2_t{ v.x * scale, v.y * scale };
}

constexpr inline vec3_t Math_Vec3( const rcommon::f32 x, const rcommon::f32 y, const rcommon::f32 z )
{
    return vec3_t{ x, y, z };
}

constexpr inline vec2_t Math_Vec2Negate( const vec2_t &v )
{
    return vec2_t{ -v.x, -v.y };
}

constexpr inline vec3_t Math_Vec3Add( const vec3_t &a, const vec3_t &b ) 
{
    return vec3_t{ a.x + b.x, a.y + b.y, a.z + b.z };
}

constexpr inline vec3_t Math_Vec3Sub( const vec3_t &a, const vec3_t &b )
{
    return vec3_t{ a.x - b.x, a.y - b.y, a.z - b.z };
}

// 

constexpr inline vec3_t Math_Vec3Scale( const vec3_t &v, const rcommon::f32 scale )
{
    return vec3_t{ v.x * scale, v.y * scale, v.z * scale };
}

constexpr inline rcommon::f32 Math_Vec3Dot( const vec3_t &a, const vec3_t &b )
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

constexpr inline vec3_t Math_Vec3FromShort( const vec3_s_t &v )
{
    return vec3_t{
        static_cast<rcommon::f32>( v.x ),
        static_cast<rcommon::f32>( v.y ),
        static_cast<rcommon::f32>( v.z )
    };
}

constexpr inline vec3_t Math_Vec3Zero( const vec3_t &v ) {
    return static_cast<vec3_t>( v ) = {};
}

constexpr inline vec3_t Math_Vec3Negate( const vec3_t &v ) {
    return vec3_t{ -v.x, -v.y, -v.z };
}

constexpr inline vec3_t Math_Vec3MA( const vec3_t &v1, const rcommon::f32 scale, const vec3_t &v2 ) {
    return vec3_t {
        v1.x + v2.x * scale,
        v1.y + v2.y * scale,  
        v1.z + v2.z * scale
    };
}

constexpr inline rcommon::f32 Math_Vec3LengthSquared( const vec3_t &v ) {
    return Math_Vec3Dot( v, v );
}

constexpr inline rcommon::f32 Math_Vec3DistanceSquared( const vec3_t &v1, const vec3_t &v2 ) {
    return Math_Vec3LengthSquared( Math_Vec3Sub( v1, v2 ) );
}

constexpr inline bool Math_Vec3Compare( const vec3_t &v1, const vec3_t &v2 ) {
    return ( v1.x == v2.x && v1.y == v2.y && v1.z == v2.z );
}

rcommon::f32 Math_Vec3Distance( const vec3_t &a, const vec3_t &b );

rcommon::f32 Math_Vec3NormalizeLength( vec3_t &v );

bool Math_Vec3Near( const vec3_t &a, const vec3_t &b, rcommon::f32 epsilon );

vec3_t Math_Vec3Lerp( const vec3_t &a, const vec3_t &b, rcommon::f32 t );

vec3_t Math_Vec3Min( const vec3_t &a, const vec3_t &b );

vec3_t Math_Vec3Max( const vec3_t &a, const vec3_t &b );

vec3_t Math_Vec3Reflect( const vec3_t &v, const vec3_t &normal );

vec3_t Math_Vec3Project( const vec3_t &v, const vec3_t &onto );

vec3_t Math_Vec3Reject( const vec3_t &v, const vec3_t &from );

vec3_t Math_Vec3Cross( const vec3_t &a, const vec3_t &b );

rcommon::f32 Math_Vec3Length( const vec3_t &v );

vec3_t Math_Vec3Normalize( const vec3_t &v );
    
}       // namespace reap::rengine::math

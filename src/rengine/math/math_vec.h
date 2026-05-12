#pragma once

#include "rengine/math/math_types.h"

namespace reap::rengine::math {

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

constexpr inline vec2_t Math_Vec2Negate( const vec2_t &v ) {
	return vec2_t{ -v.x, -v.y };
}

constexpr inline rcommon::f32 Math_Vec2Dot( const vec2_t &v1, const vec2_t &v2 ) {
	return ( v1.x * v2.x + v1.y * v2.y );
}

constexpr inline rcommon::f32 Math_Vec2LengthSquared( const vec2_t &v ) {
	return Math_Vec2Dot( v, v );
}

constexpr inline rcommon::f32 Math_Vec2DistanceSquared( const vec2_t &v1, const vec2_t &v2 ) {
	return Math_Vec2LengthSquared( Math_Vec2Sub( v1, v2 ) );
}

constexpr inline bool Math_Vec2Compare( const vec2_t &v1, const vec2_t &v2 ) {
	return v1.x == v2.x && v1.y == v2.y;
}

rcommon::f32 Math_Vec2Length( const vec2_t &v );

rcommon::f32 Math_Vec2Distance( const vec2_t &v1, const vec2_t &v2 );

vec2_t Math_Vec2Normalize( const vec2_t &v );

rcommon::f32 Math_Vec2NormalizeLength( vec2_t &v );

bool Math_Vec2Near( const vec2_t &v1, const vec2_t &v2, rcommon::f32 epsilon );

vec2_t Math_Vec2Lerp( const vec2_t &v1, const vec2_t &v2, rcommon::f32 t );

vec2_t Math_Vec2Min( const vec2_t &v1, const vec2_t &v2 );

vec2_t Math_Vec2Max( const vec2_t &v1, const vec2_t &v2 );

/*
================
Vec3 Constructors
================
*/

constexpr inline vec3_t Math_Vec3() {
	return vec3_t{};
}

constexpr inline vec3_t Math_Vec3( const rcommon::f32 x, const rcommon::f32 y, const rcommon::f32 z ) {
	return vec3_t{ x, y, z };
}

constexpr inline vec3_t Math_Vec3Zero() {
	return vec3_t{};
}

/*
================
Vec3 Operations
================
*/

constexpr inline vec3_t Math_Vec3Add( const vec3_t &v1, const vec3_t &v2 ) {
	return vec3_t{ v1.x + v2.x, v1.y + v2.y, v1.z + v2.z };
}

constexpr inline vec3_t Math_Vec3Sub( const vec3_t &v1, const vec3_t &v2 ) {
	return vec3_t{ v1.x - v2.x, v1.y - v2.y, v1.z - v2.z };
}

constexpr inline vec3_t Math_Vec3Scale( const vec3_t &v, const rcommon::f32 scale ) {
	return vec3_t{ v.x * scale, v.y * scale, v.z * scale };
}

constexpr inline rcommon::f32 Math_Vec3Dot( const vec3_t &v1, const vec3_t &v2 ) {
	return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

constexpr inline vec3_t Math_Vec3FromShort( const vec3_s_t &v ) {
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
	return vec3_t{
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

vec3_t Math_Vec3Cross( const vec3_t &v1, const vec3_t &v2 );

rcommon::f32 Math_Vec3Length( const vec3_t &v );

rcommon::f32 Math_Vec3Distance( const vec3_t &v1, const vec3_t &v2 );

vec3_t Math_Vec3Normalize( const vec3_t &v );

rcommon::f32 Math_Vec3NormalizeLength( vec3_t &v );

bool Math_Vec3Near( const vec3_t &v1, const vec3_t &v2, rcommon::f32 epsilon );

vec3_t Math_Vec3Lerp( const vec3_t &v1, const vec3_t &v2, rcommon::f32 t );

vec3_t Math_Vec3Min( const vec3_t &v1, const vec3_t &v2 );

vec3_t Math_Vec3Max( const vec3_t &v1, const vec3_t &v2 );

vec3_t Math_Vec3Reflect( const vec3_t &v, const vec3_t &normal );

vec3_t Math_Vec3Project( const vec3_t &v, const vec3_t &onto );

vec3_t Math_Vec3Reject( const vec3_t &v, const vec3_t &from );

/*
================
Vec4 Constructors
================
*/

constexpr inline vec4_t Math_Vec4()
{
    return vec4_t{};
}

constexpr inline vec4_t Math_Vec4(
    const rcommon::f32 x,
    const rcommon::f32 y,
    const rcommon::f32 z,
    const rcommon::f32 w )
{
    return vec4_t{ x, y, z, w };
}

constexpr inline vec4_t Math_Vec4Zero()
{
    return vec4_t{};
}

/*
================
Vec4 Operations
================
*/

constexpr inline vec4_t Math_Vec4Add( const vec4_t &v1, const vec4_t &v2 )
{
    return vec4_t{ v1.x + v2.x, v1.y + v2.y, v1.z + v2.z, v1.w + v2.w };
}

constexpr inline vec4_t Math_Vec4Sub( const vec4_t &v1, const vec4_t &v2 )
{
    return vec4_t{ v1.x - v2.x, v1.y - v2.y, v1.z - v2.z, v1.w - v2.w };
}

constexpr inline vec4_t Math_Vec4Scale( const vec4_t &v, const rcommon::f32 scale )
{
    return vec4_t{ v.x * scale, v.y * scale, v.z * scale, v.w * scale };
}

constexpr inline vec4_t Math_Vec4Negate( const vec4_t &v )
{
    return vec4_t{ -v.x, -v.y, -v.z, -v.w };
}

constexpr inline rcommon::f32 Math_Vec4Dot( const vec4_t &v1, const vec4_t &v2 )
{
    return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z + v1.w * v2.w;
}

constexpr inline rcommon::f32 Math_Vec4LengthSquared( const vec4_t &v )
{
    return Math_Vec4Dot( v, v );
}

constexpr inline bool Math_Vec4Compare( const vec4_t &v1, const vec4_t &v2 )
{
    return v1.x == v2.x && v1.y == v2.y && v1.z == v2.z && v1.w == v2.w;
}

rcommon::f32 Math_Vec4Length( const vec4_t &v );

vec4_t Math_Vec4Normalize( const vec4_t &v );

bool Math_Vec4Near( const vec4_t &v1, const vec4_t &v2, rcommon::f32 epsilon );

vec4_t Math_Vec4Lerp( const vec4_t &v1, const vec4_t &v2, rcommon::f32 t );

vec4_t Math_Vec4Min( const vec4_t &v1, const vec4_t &v2 );

vec4_t Math_Vec4Max( const vec4_t &v1, const vec4_t &v2 );

} // namespace reap::rengine::math

#ifndef CYPHER_COMMON_TIER0_BASETYPES_H
#define CYPHER_COMMON_TIER0_BASETYPES_H
#pragma once

/*
================
CypherCommon Base Types

Primitive engine-wide scalar types and constants.

Rules:
- No CypherEngine dependency.
- No allocation.
- No containers.
- No platform API calls.
- No subsystem policy.
================
*/

#include <cstddef>
#include <cstdint>
#include <limits>
#include <type_traits>

namespace cypher::common
{

/*
================
Fixed Width Integer Types
================
*/
using i8 = std::int8_t;
using i16 = std::int16_t;
using i32 = std::int32_t;
using i64 = std::int64_t;

using u8 = std::uint8_t;
using u16 = std::uint16_t;
using u32 = std::uint32_t;
using u64 = std::uint64_t;

/*
================
Floating Point Types
================
*/
using f32 = float;
using f64 = double;

/*
================
Size And Pointer Types
================
*/
using usize = std::size_t;
using isize = std::ptrdiff_t;
using uintptr = std::uintptr_t;
using intptr = std::intptr_t;

/*
================
Byte And Text Pointer Types
================
*/
using byte = u8;
using char8 = char;
using cstring = const char *;
using mstring = char *;

/*
================
Boolean Types
================
*/
using bool_t = bool;

enum class b8 : u8 {
    False = 0u,
    True = 1u
};

constexpr bool_t CY_FALSE = false;
constexpr bool_t CY_TRUE = true;

/*
================
Generic Engine ID Types
================
*/
using index_t = u32;
using generation_t = u32;
using handle_t = u32;
using frame_index_t = u64;

constexpr index_t CY_INVALID_INDEX = std::numeric_limits<index_t>::max();
constexpr generation_t CY_INVALID_GENERATION = std::numeric_limits<generation_t>::max();
constexpr handle_t CY_INVALID_HANDLE = 0u;
constexpr frame_index_t CY_INVALID_FRAME_INDEX = std::numeric_limits<frame_index_t>::max();

/*
================
Common Size Constants
================
*/
constexpr usize CY_KB = 1024u;
constexpr usize CY_MB = CY_KB * 1024u;
constexpr usize CY_GB = CY_MB * 1024u;

/*
================
Layout Checks
================
*/
static_assert( sizeof( i8 ) == 1, "i8 must be 1 byte." );
static_assert( sizeof( i16 ) == 2, "i16 must be 2 bytes." );
static_assert( sizeof( i32 ) == 4, "i32 must be 4 bytes." );
static_assert( sizeof( i64 ) == 8, "i64 must be 8 bytes." );

static_assert( sizeof( u8 ) == 1, "u8 must be 1 byte." );
static_assert( sizeof( u16 ) == 2, "u16 must be 2 bytes." );
static_assert( sizeof( u32 ) == 4, "u32 must be 4 bytes." );
static_assert( sizeof( u64 ) == 8, "u64 must be 8 bytes." );

static_assert( sizeof( f32 ) == 4, "f32 must be 4 bytes." );
static_assert( sizeof( f64 ) == 8, "f64 must be 8 bytes." );

static_assert( sizeof( byte ) == 1, "byte must be 1 byte." );
static_assert( sizeof( b8 ) == 1, "b8 must be 1 byte." );
static_assert( std::is_signed_v<i8>, "i8 must be signed." );
static_assert( std::is_unsigned_v<u8>, "u8 must be unsigned." );

} // namespace cypher::common

#endif // CYPHER_COMMON_TIER0_BASETYPES_H

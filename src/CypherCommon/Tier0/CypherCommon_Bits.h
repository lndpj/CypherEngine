#ifndef CYPHER_COMMON_TIER0_BITS_H
#define CYPHER_COMMON_TIER0_BITS_H
#pragma once

/*
================
CypherCommon Bits

Typed bit helpers for flags, masks, handles, packet fields and allocators.
================
*/

#include "CypherCommon_BaseTypes.h"

#include <bit>

namespace cypher::common
{

constexpr u32 Bit32( u32 bit )
{
    return 1u << bit;
}

constexpr u64 Bit64( u32 bit )
{
    return 1ull << bit;
}

constexpr bool_t HasAnyFlags( u32 value, u32 flags )
{
    return ( value & flags ) != 0u;
}

constexpr bool_t HasAllFlags( u32 value, u32 flags )
{
    return ( value & flags ) == flags;
}

constexpr u32 SetFlags( u32 value, u32 flags )
{
    return value | flags;
}

constexpr u32 ClearFlags( u32 value, u32 flags )
{
    return value & ~flags;
}

constexpr u32 ToggleFlags( u32 value, u32 flags )
{
    return value ^ flags;
}

inline i32 PopCount32( u32 value )
{
    return static_cast<i32>( std::popcount( value ) );
}

inline i32 PopCount64( u64 value )
{
    return static_cast<i32>( std::popcount( value ) );
}

inline i32 CountLeadingZeros32( u32 value )
{
    return static_cast<i32>( std::countl_zero( value ) );
}

inline i32 CountLeadingZeros64( u64 value )
{
    return static_cast<i32>( std::countl_zero( value ) );
}

inline i32 CountTrailingZeros32( u32 value )
{
    return static_cast<i32>( std::countr_zero( value ) );
}

inline i32 CountTrailingZeros64( u64 value )
{
    return static_cast<i32>( std::countr_zero( value ) );
}

constexpr u32 RotateLeft32( u32 value, i32 shift )
{
    return std::rotl( value, shift );
}

constexpr u32 RotateRight32( u32 value, i32 shift )
{
    return std::rotr( value, shift );
}

constexpr u64 RotateLeft64( u64 value, i32 shift )
{
    return std::rotl( value, shift );
}

constexpr u64 RotateRight64( u64 value, i32 shift )
{
    return std::rotr( value, shift );
}

} // namespace cypher::common

#endif // CYPHER_COMMON_TIER0_BITS_H

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

// Builds a 32-bit mask with one bit set.
constexpr u32 Bit32( u32 bit )
{
    return 1u << bit;
}

// Builds a 64-bit mask with one bit set.
constexpr u64 Bit64( u32 bit )
{
    return 1ull << bit;
}

// Returns true when any requested flag is set.
constexpr bool_t HasAnyFlags( u32 value, u32 flags )
{
    return ( value & flags ) != 0u;
}

// Returns true when all requested flags are set.
constexpr bool_t HasAllFlags( u32 value, u32 flags )
{
    return ( value & flags ) == flags;
}

// Returns value with flags enabled.
constexpr u32 SetFlags( u32 value, u32 flags )
{
    return value | flags;
}

// Returns value with flags disabled.
constexpr u32 ClearFlags( u32 value, u32 flags )
{
    return value & ~flags;
}

// Returns value with flags flipped.
constexpr u32 ToggleFlags( u32 value, u32 flags )
{
    return value ^ flags;
}

// Counts set bits in a 32-bit value.
inline i32 PopCount32( u32 value )
{
    return static_cast<i32>( std::popcount( value ) );
}

// Counts set bits in a 64-bit value.
inline i32 PopCount64( u64 value )
{
    return static_cast<i32>( std::popcount( value ) );
}

// Counts zero bits before the highest set 32-bit bit.
inline i32 CountLeadingZeros32( u32 value )
{
    return static_cast<i32>( std::countl_zero( value ) );
}

// Counts zero bits before the highest set 64-bit bit.
inline i32 CountLeadingZeros64( u64 value )
{
    return static_cast<i32>( std::countl_zero( value ) );
}

// Counts zero bits after the lowest set 32-bit bit.
inline i32 CountTrailingZeros32( u32 value )
{
    return static_cast<i32>( std::countr_zero( value ) );
}

// Counts zero bits after the lowest set 64-bit bit.
inline i32 CountTrailingZeros64( u64 value )
{
    return static_cast<i32>( std::countr_zero( value ) );
}

// Rotates a 32-bit value left by shift bits.
constexpr u32 RotateLeft32( u32 value, i32 shift )
{
    return std::rotl( value, shift );
}

// Rotates a 32-bit value right by shift bits.
constexpr u32 RotateRight32( u32 value, i32 shift )
{
    return std::rotr( value, shift );
}

// Rotates a 64-bit value left by shift bits.
constexpr u64 RotateLeft64( u64 value, i32 shift )
{
    return std::rotl( value, shift );
}

// Rotates a 64-bit value right by shift bits.
constexpr u64 RotateRight64( u64 value, i32 shift )
{
    return std::rotr( value, shift );
}

} // namespace cypher::common

#endif // CYPHER_COMMON_TIER0_BITS_H

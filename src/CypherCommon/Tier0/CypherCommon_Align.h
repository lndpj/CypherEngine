#ifndef CYPHER_COMMON_TIER0_ALIGN_H
#define CYPHER_COMMON_TIER0_ALIGN_H
#pragma once

/*
================
CypherCommon Align

Power-of-two alignment helpers for memory, binary file formats and GPU data.
================
*/

#include "CypherCommon_BaseTypes.h"

namespace cypher::common
{

// Returns true when value is a non-zero power of two.
constexpr bool_t IsPowerOfTwo( usize value )
{
    return value != 0u && ( value & ( value - 1u ) ) == 0u;
}

// Rounds value up to the next alignment boundary.
constexpr usize AlignUp( usize value, usize alignment )
{
    return ( value + ( alignment - 1u ) ) & ~( alignment - 1u );
}

// Rounds value down to the previous alignment boundary.
constexpr usize AlignDown( usize value, usize alignment )
{
    return value & ~( alignment - 1u );
}

// Returns true when value already satisfies alignment.
constexpr bool_t IsAligned( usize value, usize alignment )
{
    return ( value & ( alignment - 1u ) ) == 0u;
}

// Rounds a writable pointer up to the next alignment boundary.
inline void *AlignPointerUp( void *ptr, usize alignment )
{
    return reinterpret_cast<void *>( AlignUp( reinterpret_cast<uintptr>( ptr ), alignment ) );
}

// Rounds a read-only pointer up to the next alignment boundary.
inline const void *AlignPointerUp( const void *ptr, usize alignment )
{
    return reinterpret_cast<const void *>( AlignUp( reinterpret_cast<uintptr>( ptr ), alignment ) );
}

// Rounds a writable pointer down to the previous alignment boundary.
inline void *AlignPointerDown( void *ptr, usize alignment )
{
    return reinterpret_cast<void *>( AlignDown( reinterpret_cast<uintptr>( ptr ), alignment ) );
}

// Rounds a read-only pointer down to the previous alignment boundary.
inline const void *AlignPointerDown( const void *ptr, usize alignment )
{
    return reinterpret_cast<const void *>( AlignDown( reinterpret_cast<uintptr>( ptr ), alignment ) );
}

// Returns true when ptr satisfies alignment.
inline bool_t IsPointerAligned( const void *ptr, usize alignment )
{
    return IsAligned( reinterpret_cast<uintptr>( ptr ), alignment );
}

} // namespace cypher::common

#endif // CYPHER_COMMON_TIER0_ALIGN_H

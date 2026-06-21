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

constexpr bool_t IsPowerOfTwo( usize value )
{
    return value != 0u && ( value & ( value - 1u ) ) == 0u;
}

constexpr usize AlignUp( usize value, usize alignment )
{
    return ( value + ( alignment - 1u ) ) & ~( alignment - 1u );
}

constexpr usize AlignDown( usize value, usize alignment )
{
    return value & ~( alignment - 1u );
}

constexpr bool_t IsAligned( usize value, usize alignment )
{
    return ( value & ( alignment - 1u ) ) == 0u;
}

inline void *AlignPointerUp( void *ptr, usize alignment )
{
    return reinterpret_cast<void *>( AlignUp( reinterpret_cast<uintptr>( ptr ), alignment ) );
}

inline const void *AlignPointerUp( const void *ptr, usize alignment )
{
    return reinterpret_cast<const void *>( AlignUp( reinterpret_cast<uintptr>( ptr ), alignment ) );
}

inline void *AlignPointerDown( void *ptr, usize alignment )
{
    return reinterpret_cast<void *>( AlignDown( reinterpret_cast<uintptr>( ptr ), alignment ) );
}

inline const void *AlignPointerDown( const void *ptr, usize alignment )
{
    return reinterpret_cast<const void *>( AlignDown( reinterpret_cast<uintptr>( ptr ), alignment ) );
}

inline bool_t IsPointerAligned( const void *ptr, usize alignment )
{
    return IsAligned( reinterpret_cast<uintptr>( ptr ), alignment );
}

} // namespace cypher::common

#endif // CYPHER_COMMON_TIER0_ALIGN_H

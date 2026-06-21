#ifndef CYPHER_COMMON_TIER0_MEMORYOPS_H
#define CYPHER_COMMON_TIER0_MEMORYOPS_H
#pragma once

/*
================
CypherCommon Memory Ops

Raw byte memory operations. This is not an allocator layer.
================
*/

#include "CypherCommon_BaseTypes.h"

#include <cstring>
#include <type_traits>

namespace cypher::common
{

// Copies byte_count bytes from src to dst; ranges must not overlap.
inline void *MemCopy( void *dst, const void *src, usize byte_count )
{
    if ( byte_count == 0u ) {
        return dst;
    }
    return std::memcpy( dst, src, byte_count );
}

// Moves byte_count bytes from src to dst; ranges may overlap.
inline void *MemMove( void *dst, const void *src, usize byte_count )
{
    if ( byte_count == 0u ) {
        return dst;
    }
    return std::memmove( dst, src, byte_count );
}

// Fills byte_count bytes at dst with value.
inline void *MemSet( void *dst, i32 value, usize byte_count )
{
    if ( byte_count == 0u ) {
        return dst;
    }
    return std::memset( dst, value, byte_count );
}

// Clears byte_count bytes at dst to zero.
inline void *MemZero( void *dst, usize byte_count )
{
    return MemSet( dst, 0, byte_count );
}

// Compares two byte ranges like memcmp.
inline i32 MemCompare( const void *a, const void *b, usize byte_count )
{
    if ( byte_count == 0u ) {
        return 0;
    }
    return std::memcmp( a, b, byte_count );
}

// Returns true when both byte ranges are identical.
inline bool_t MemEqual( const void *a, const void *b, usize byte_count )
{
    return MemCompare( a, b, byte_count ) == 0;
}

// Clears a trivially copyable object to zero bytes.
template <typename type_t>
inline void ZeroStruct( type_t &value )
{
    static_assert( std::is_trivially_copyable_v<type_t>, "ZeroStruct requires a trivially copyable type." );
    MemZero( &value, sizeof( value ) );
}

// Clears a fixed-size array of trivially copyable objects.
template <typename type_t, usize count>
inline void ZeroArray( type_t ( &values )[count] )
{
    static_assert( std::is_trivially_copyable_v<type_t>, "ZeroArray requires a trivially copyable type." );
    MemZero( values, sizeof( values ) );
}

} // namespace cypher::common

#endif // CYPHER_COMMON_TIER0_MEMORYOPS_H

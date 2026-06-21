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

inline void *MemCopy( void *dst, const void *src, usize byte_count )
{
    if ( byte_count == 0u ) {
        return dst;
    }
    return std::memcpy( dst, src, byte_count );
}

inline void *MemMove( void *dst, const void *src, usize byte_count )
{
    if ( byte_count == 0u ) {
        return dst;
    }
    return std::memmove( dst, src, byte_count );
}

inline void *MemSet( void *dst, i32 value, usize byte_count )
{
    if ( byte_count == 0u ) {
        return dst;
    }
    return std::memset( dst, value, byte_count );
}

inline void *MemZero( void *dst, usize byte_count )
{
    return MemSet( dst, 0, byte_count );
}

inline i32 MemCompare( const void *a, const void *b, usize byte_count )
{
    if ( byte_count == 0u ) {
        return 0;
    }
    return std::memcmp( a, b, byte_count );
}

inline bool_t MemEqual( const void *a, const void *b, usize byte_count )
{
    return MemCompare( a, b, byte_count ) == 0;
}

template <typename type_t>
inline void ZeroStruct( type_t &value )
{
    static_assert( std::is_trivially_copyable_v<type_t>, "ZeroStruct requires a trivially copyable type." );
    MemZero( &value, sizeof( value ) );
}

template <typename type_t, usize count>
inline void ZeroArray( type_t ( &values )[count] )
{
    static_assert( std::is_trivially_copyable_v<type_t>, "ZeroArray requires a trivially copyable type." );
    MemZero( values, sizeof( values ) );
}

} // namespace cypher::common

#endif // CYPHER_COMMON_TIER0_MEMORYOPS_H

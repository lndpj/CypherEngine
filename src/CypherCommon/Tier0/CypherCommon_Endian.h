#ifndef CYPHER_COMMON_TIER0_ENDIAN_H
#define CYPHER_COMMON_TIER0_ENDIAN_H
#pragma once

/*
================
CypherCommon Endian

Byte swapping, host/endian conversion and FourCC helpers.
================
*/

#include "CypherCommon_BaseTypes.h"
#include "CypherCommon_Platform.h"

namespace cypher::common
{

constexpr u16 ByteSwap16( u16 value )
{
    return static_cast<u16>( ( value >> 8u ) | ( value << 8u ) );
}

constexpr u32 ByteSwap32( u32 value )
{
    return ( ( value & 0x000000FFu ) << 24u ) |
           ( ( value & 0x0000FF00u ) << 8u ) |
           ( ( value & 0x00FF0000u ) >> 8u ) |
           ( ( value & 0xFF000000u ) >> 24u );
}

constexpr u64 ByteSwap64( u64 value )
{
    return ( ( value & 0x00000000000000FFull ) << 56u ) |
           ( ( value & 0x000000000000FF00ull ) << 40u ) |
           ( ( value & 0x0000000000FF0000ull ) << 24u ) |
           ( ( value & 0x00000000FF000000ull ) << 8u ) |
           ( ( value & 0x000000FF00000000ull ) >> 8u ) |
           ( ( value & 0x0000FF0000000000ull ) >> 24u ) |
           ( ( value & 0x00FF000000000000ull ) >> 40u ) |
           ( ( value & 0xFF00000000000000ull ) >> 56u );
}

constexpr u32 MakeFourCC( char a, char b, char c, char d )
{
    return static_cast<u32>( static_cast<u8>( a ) ) |
           ( static_cast<u32>( static_cast<u8>( b ) ) << 8u ) |
           ( static_cast<u32>( static_cast<u8>( c ) ) << 16u ) |
           ( static_cast<u32>( static_cast<u8>( d ) ) << 24u );
}

constexpr u16 HostToLittle16( u16 value )
{
#if CYPHER_ENDIAN_LITTLE
    return value;
#else
    return ByteSwap16( value );
#endif
}

constexpr u32 HostToLittle32( u32 value )
{
#if CYPHER_ENDIAN_LITTLE
    return value;
#else
    return ByteSwap32( value );
#endif
}

constexpr u64 HostToLittle64( u64 value )
{
#if CYPHER_ENDIAN_LITTLE
    return value;
#else
    return ByteSwap64( value );
#endif
}

constexpr u16 LittleToHost16( u16 value )
{
    return HostToLittle16( value );
}

constexpr u32 LittleToHost32( u32 value )
{
    return HostToLittle32( value );
}

constexpr u64 LittleToHost64( u64 value )
{
    return HostToLittle64( value );
}

constexpr u16 HostToBig16( u16 value )
{
#if CYPHER_ENDIAN_BIG
    return value;
#else
    return ByteSwap16( value );
#endif
}

constexpr u32 HostToBig32( u32 value )
{
#if CYPHER_ENDIAN_BIG
    return value;
#else
    return ByteSwap32( value );
#endif
}

constexpr u64 HostToBig64( u64 value )
{
#if CYPHER_ENDIAN_BIG
    return value;
#else
    return ByteSwap64( value );
#endif
}

constexpr u16 BigToHost16( u16 value )
{
    return HostToBig16( value );
}

constexpr u32 BigToHost32( u32 value )
{
    return HostToBig32( value );
}

constexpr u64 BigToHost64( u64 value )
{
    return HostToBig64( value );
}

} // namespace cypher::common

#endif // CYPHER_COMMON_TIER0_ENDIAN_H

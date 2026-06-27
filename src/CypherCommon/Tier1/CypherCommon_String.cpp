/*======================================================================
   File: CypherCommon_String.cpp
   Project: CYPHER
   Author: ksiric <email@example.com>
   Created: 2026-06-22 18:00:51
   Last Modified by: ksiric
   Last Modified: 2026-06-22 22:31:14
   ---------------------------------------------------------------------
   Description:

   ---------------------------------------------------------------------
   License:
   Company:
   Version: 0.1.0
 ======================================================================
                                                                       */

#include "CypherCommon_String.h"
#include "CypherCommon_Char.h"

namespace
{

CYPHER_FORCE_INLINE cypher::common::i32 CyCompareBytes( cypher::common::u8 chA, cypher::common::u8 chB )
{
    return static_cast<cypher::common::i32>( chA ) - static_cast<cypher::common::i32>( chB );
}

} // namespace

namespace cypher::common
{

usize Cy_strlen( const char *pString )
{
    if ( pString == nullptr ) {
        return 0u;
    }

    usize cchCount = 0u;
    const char *pCursor = pString;
    while ( *pCursor != '\0' ) {
        ++pCursor;
        ++cchCount;
    }

    return cchCount;
}

usize Cy_strnlen( const char *pString, usize cchMax )
{
    if ( pString == nullptr ) {
        return 0u;
    }

    usize cchCount = 0u;
    const char *pCursor = pString;
    while ( *pCursor != '\0' && cchCount < cchMax ) {
        ++pCursor;
        ++cchCount;
    }

    return cchCount;
}

bool_t Cy_strisempty( const char *pString )
{
    if ( pString == nullptr ) {
        return true;
    }

    return pString[0] == '\0';
}

bool_t Cy_strisblank( const char *pString )
{
    if ( pString == nullptr ) {
        return true;
    }

    const char *pCursor = pString;
    while ( *pCursor != '\0' ) {
        if ( !Char_IsWhitespaceAscii( *pCursor ) ) {
            return false;
        }
        ++pCursor;
    }

    return true;
}

i32 Cy_strcmp( const char *pStringA, const char *pStringB )
{
    if ( pStringA == pStringB ) {
        return 0;
    }

    const char *pA = pStringA != nullptr ? pStringA : "";
    const char *pB = pStringB != nullptr ? pStringB : "";

    while ( true ) {
        u8 chA = static_cast<u8>( pA[0] );
        u8 chB = static_cast<u8>( pB[0] );
        if ( chA != chB || chA == 0u ) {
            return CyCompareBytes( chA, chB );
        }

        chA = static_cast<u8>( pA[1] );
        chB = static_cast<u8>( pB[1] );
        if ( chA != chB || chA == 0u ) {
            return CyCompareBytes( chA, chB );
        }

        chA = static_cast<u8>( pA[2] );
        chB = static_cast<u8>( pB[2] );
        if ( chA != chB || chA == 0u ) {
            return CyCompareBytes( chA, chB );
        }

        chA = static_cast<u8>( pA[3] );
        chB = static_cast<u8>( pB[3] );
        if ( chA != chB || chA == 0u ) {
            return CyCompareBytes( chA, chB );
        }

        pA += 4u;
        pB += 4u;
    }
}

i32 Cy_strncmp( const char *pStringA, const char *pStringB, usize cchMax )
{
    if ( cchMax == 0u || pStringA == pStringB ) {
        return 0;
    }

    const char *pA = pStringA != nullptr ? pStringA : "";
    const char *pB = pStringB != nullptr ? pStringB : "";

    usize cchRemaining = cchMax;
    while ( cchRemaining >= 4u ) {
        u8 chA = static_cast<u8>( pA[0] );
        u8 chB = static_cast<u8>( pB[0] );
        if ( chA != chB || chA == 0u ) {
            return CyCompareBytes( chA, chB );
        }

        chA = static_cast<u8>( pA[1] );
        chB = static_cast<u8>( pB[1] );
        if ( chA != chB || chA == 0u ) {
            return CyCompareBytes( chA, chB );
        }

        chA = static_cast<u8>( pA[2] );
        chB = static_cast<u8>( pB[2] );
        if ( chA != chB || chA == 0u ) {
            return CyCompareBytes( chA, chB );
        }

        chA = static_cast<u8>( pA[3] );
        chB = static_cast<u8>( pB[3] );
        if ( chA != chB || chA == 0u ) {
            return CyCompareBytes( chA, chB );
        }

        pA += 4u;
        pB += 4u;
        cchRemaining -= 4u;
    }

    while ( cchRemaining > 0u ) {
        const u8 chA = static_cast<u8>( *pA );
        const u8 chB = static_cast<u8>( *pB );
        if ( chA != chB || chA == 0u ) {
            return CyCompareBytes( chA, chB );
        }

        ++pA;
        ++pB;
        --cchRemaining;
    }

    return 0;
}

i32 Cy_stricmp( const char *pStringA, const char *pStringB )
{
    const char *pA = pStringA != nullptr ? pStringA : "";
    const char *pB = pStringB != nullptr ? pStringB : "";
    while ( true ) {
        const u8 chA = static_cast<u8>( Char_ToLowerAscii( *pA ) );
        const u8 chB = static_cast<u8>( Char_ToLowerAscii( *pB ) );
        if ( chA != chB ) {
            return static_cast<i32>( chA ) - static_cast<i32>( chB );
        }
        if ( chA == 0u ) {
            return 0;
        }
        ++pA;
        ++pB;
    }
    return ( 0 );
}

i32 Cy_strnicmp( const char *pStringA, const char *pStringB, usize cchMax )
{
    const char *pA = pStringA != nullptr ? pStringA : "";
    const char *pB = pStringB != nullptr ? pStringB : "";
    if ( cchMax == 0u ) {
        return 0;
    }
    usize cchCount = 0u;
    while ( cchCount < cchMax ) {
        const u8 chA = static_cast<u8>( Char_ToLowerAscii( *pA ) );
        const u8 chB = static_cast<u8>( Char_ToLowerAscii( *pB ) );
        if ( chA != chB ) {
            return static_cast<i32>( chA ) - static_cast<i32>( chB );
        }
        if ( chA == 0u ) {
            return 0;
        }
        ++pA;
        ++pB;
        ++cchCount;
    }
    return ( 0 );
}

bool_t Cy_strequal( const char *pStringA, const char *pStringB )
{
    return Cy_strcmp( pStringA, pStringB ) == 0;
}

bool_t Cy_striequal( const char *pStringA, const char *pStringB )
{
    return Cy_stricmp( pStringA, pStringB ) == 0;
}

bool_t Cy_strnequal( const char *pStringA, const char *pStringB, usize cchMax )
{
    return Cy_strncmp( pStringA, pStringB, cchMax ) == 0;
}

bool_t Cy_strniequal( const char *pStringA, const char *pStringB, usize cchMax )
{
    return Cy_strnicmp( pStringA, pStringB, cchMax ) == 0;
}

usize Cy_strncpy( char *pDest, const char *pSrc, usize cchDest )
{
    const char *pRead = pSrc != nullptr ? pSrc : "";
    usize cchSource = 0u;

    if ( pDest != nullptr && cchDest > 0u ) {
        usize cchWrite = 0u;
        while ( pRead[cchSource] != '\0' ) {
            if ( cchWrite + 1u < cchDest ) {
                pDest[cchWrite] = pRead[cchSource];
                ++cchWrite;
            }
            ++cchSource;
        }
        pDest[cchWrite] = '\0';
        return cchSource;
    }

    while ( pRead[cchSource] != '\0' ) {
        ++cchSource;
    }

    return cchSource;
}

} // namespace cypher::common

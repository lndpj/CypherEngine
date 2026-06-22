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
    const char *pA = pStringA != nullptr ? pStringA : "";
    const char *pB = pStringB != nullptr ? pStringB : "";

    while ( *pA != '\0' && *pA == *pB ) {
        ++pA;
        ++pB;
    }

    const u8 chA = static_cast<u8>( *pA );
    const u8 chB = static_cast<u8>( *pB );
    return static_cast<i32>( chA ) - static_cast<i32>( chB );
}

i32 Cy_strncmp( const char *pStringA, const char *pStringB, usize cchMax )
{
    const char *pA = pStringA != nullptr ? pStringA : "";
    const char *pB = pStringB != nullptr ? pStringB : "";
    if ( cchMax == 0u ) {
        return 0;
    }
    usize cchCount = 0u;
    while ( cchCount < cchMax ) {
        if ( *pA != *pB ) {
            // @NOTE: converting here because of potential compiler platform issues.
            const u8 chA = static_cast<u8>( *pA );
            const u8 chB = static_cast<u8>( *pB );
            return ( static_cast<i32>( chA ) - static_cast<i32>( chB ) );
        }
        if ( *pA == '\0' ) {
            return 0;
        }
        ++pA;
        ++pB;
        ++cchCount;
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
}

} // namespace cypher::common

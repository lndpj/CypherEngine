#include "CypherCommon_String.h"

#include <catch2/catch_test_macros.hpp>

#include <cstring>

using namespace cypher::common;

namespace
{

i32 SignOf( i32 nValue )
{
    if ( nValue < 0 ) {
        return -1;
    }
    if ( nValue > 0 ) {
        return 1;
    }
    return 0;
}

} // namespace

TEST_CASE( "Cy_strlen returns zero for null and empty strings", "[CypherCommon][Tier1][String]" )
{
    REQUIRE( Cy_strlen( nullptr ) == 0u );
    REQUIRE( Cy_strlen( "" ) == 0u );
}

TEST_CASE( "Cy_strlen counts characters before the terminator", "[CypherCommon][Tier1][String]" )
{
    REQUIRE( Cy_strlen( "cypher" ) == 6u );
    REQUIRE( Cy_strlen( "cypher engine" ) == 13u );
}

TEST_CASE( "Cy_strnlen caps the scan at the requested maximum", "[CypherCommon][Tier1][String]" )
{
    REQUIRE( Cy_strnlen( nullptr, 8u ) == 0u );
    REQUIRE( Cy_strnlen( "cypher", 0u ) == 0u );
    REQUIRE( Cy_strnlen( "cypher", 3u ) == 3u );
    REQUIRE( Cy_strnlen( "cypher", 64u ) == 6u );
}

TEST_CASE( "Cy_strisempty treats null as empty", "[CypherCommon][Tier1][String]" )
{
    REQUIRE( Cy_strisempty( nullptr ) );
    REQUIRE( Cy_strisempty( "" ) );
    REQUIRE_FALSE( Cy_strisempty( "cypher" ) );
}

TEST_CASE( "Cy_strisblank accepts only empty or ASCII whitespace strings", "[CypherCommon][Tier1][String]" )
{
    REQUIRE( Cy_strisblank( nullptr ) );
    REQUIRE( Cy_strisblank( "" ) );
    REQUIRE( Cy_strisblank( " \t\r\n" ) );
    REQUIRE_FALSE( Cy_strisblank( " cypher " ) );
}

TEST_CASE( "Cy_strcmp compares null as an empty string", "[CypherCommon][Tier1][String]" )
{
    REQUIRE( Cy_strcmp( nullptr, nullptr ) == 0 );
    REQUIRE( Cy_strcmp( nullptr, "" ) == 0 );
    REQUIRE( Cy_strcmp( "abc", "abc" ) == 0 );
    REQUIRE( Cy_strcmp( "abc", "abd" ) < 0 );
    REQUIRE( Cy_strcmp( "abd", "abc" ) > 0 );
    REQUIRE( Cy_strcmp( "abc", "ab" ) > 0 );
    REQUIRE( Cy_strcmp( "ab", "abc" ) < 0 );
    REQUIRE( Cy_strcmp( "Apple", "apple" ) < 0 );
}

TEST_CASE( "Cy_strcmp matches unsigned byte ordering across mismatch positions", "[CypherCommon][Tier1][String]" )
{
    const char pEqualA[] = "abcdefghijklmnopqrstuvwxyz0123456789";
    const char pEqualB[] = "abcdefghijklmnopqrstuvwxyz0123456789";
    const char pEarlyA[] = "xbcdefghijklmnopqrstuvwxyz0123456789";
    const char pEarlyB[] = "abcdefghijklmnopqrstuvwxyz0123456789";
    const char pMiddleA[] = "abcdefghijklMnopqrstuvwxyz0123456789";
    const char pMiddleB[] = "abcdefghijklmnopqrstuvwxyz0123456789";
    const char pLateA[] = "abcdefghijklmnopqrstuvwxyz012345678X";
    const char pLateB[] = "abcdefghijklmnopqrstuvwxyz0123456789";
    const char pHighA[] = { static_cast<char>( 0x80u ), '\0' };
    const char pHighB[] = { static_cast<char>( 0x7Fu ), '\0' };

    REQUIRE( SignOf( Cy_strcmp( pEqualA, pEqualB ) ) == SignOf( std::strcmp( pEqualA, pEqualB ) ) );
    REQUIRE( SignOf( Cy_strcmp( pEarlyA, pEarlyB ) ) == SignOf( std::strcmp( pEarlyA, pEarlyB ) ) );
    REQUIRE( SignOf( Cy_strcmp( pMiddleA, pMiddleB ) ) == SignOf( std::strcmp( pMiddleA, pMiddleB ) ) );
    REQUIRE( SignOf( Cy_strcmp( pLateA, pLateB ) ) == SignOf( std::strcmp( pLateA, pLateB ) ) );
    REQUIRE( SignOf( Cy_strcmp( pHighA, pHighB ) ) == SignOf( std::strcmp( pHighA, pHighB ) ) );
}

TEST_CASE( "Cy_strncmp respects the maximum character count", "[CypherCommon][Tier1][String]" )
{
    REQUIRE( Cy_strncmp( "abc", "xyz", 0u ) == 0 );
    REQUIRE( Cy_strncmp( "abc", "abd", 2u ) == 0 );
    REQUIRE( Cy_strncmp( "abc", "abd", 3u ) < 0 );
    REQUIRE( Cy_strncmp( "abd", "abc", 3u ) > 0 );
    REQUIRE( Cy_strncmp( "ab", "abc", 2u ) == 0 );
    REQUIRE( Cy_strncmp( "ab", "abc", 3u ) < 0 );
}

TEST_CASE( "Cy_strncmp matches unsigned byte ordering for capped comparisons", "[CypherCommon][Tier1][String]" )
{
    const char pStringA[] = "textures/world/industrial/wall_panel_01_albedo.dds";
    const char pStringB[] = "textures/world/industrial/wall_panel_01_normal.dds";
    const char pHighA[] = { 'a', static_cast<char>( 0x80u ), '\0' };
    const char pHighB[] = { 'a', static_cast<char>( 0x7Fu ), '\0' };

    REQUIRE( SignOf( Cy_strncmp( pStringA, pStringB, 0u ) ) == SignOf( std::strncmp( pStringA, pStringB, 0u ) ) );
    REQUIRE( SignOf( Cy_strncmp( pStringA, pStringB, 16u ) ) == SignOf( std::strncmp( pStringA, pStringB, 16u ) ) );
    REQUIRE( SignOf( Cy_strncmp( pStringA, pStringB, 40u ) ) == SignOf( std::strncmp( pStringA, pStringB, 40u ) ) );
    REQUIRE( SignOf( Cy_strncmp( pHighA, pHighB, 3u ) ) == SignOf( std::strncmp( pHighA, pHighB, 3u ) ) );
}

TEST_CASE( "Cy_stricmp compares ASCII strings ignoring case", "[CypherCommon][Tier1][String]" )
{
    REQUIRE( Cy_stricmp( nullptr, nullptr ) == 0 );
    REQUIRE( Cy_stricmp( nullptr, "" ) == 0 );
    REQUIRE( Cy_stricmp( "Player", "player" ) == 0 );
    REQUIRE( Cy_stricmp( "ABC", "abc" ) == 0 );
    REQUIRE( Cy_stricmp( "abc", "abd" ) < 0 );
    REQUIRE( Cy_stricmp( "abd", "abc" ) > 0 );
    REQUIRE( Cy_stricmp( "", "A" ) < 0 );
}

TEST_CASE( "Cy_strnicmp compares ASCII strings ignoring case up to a maximum", "[CypherCommon][Tier1][String]" )
{
    REQUIRE( Cy_strnicmp( "ABC", "abc", 3u ) == 0 );
    REQUIRE( Cy_strnicmp( "ABC", "abd", 2u ) == 0 );
    REQUIRE( Cy_strnicmp( "ABC", "abd", 3u ) < 0 );
    REQUIRE( Cy_strnicmp( "ABD", "abc", 3u ) > 0 );
    REQUIRE( Cy_strnicmp( "ab", "ABC", 2u ) == 0 );
    REQUIRE( Cy_strnicmp( "ab", "ABC", 3u ) < 0 );
}

TEST_CASE( "Cy string equality helpers wrap the compare functions", "[CypherCommon][Tier1][String]" )
{
    REQUIRE( Cy_strequal( "abc", "abc" ) );
    REQUIRE_FALSE( Cy_strequal( "ABC", "abc" ) );
    REQUIRE( Cy_striequal( "ABC", "abc" ) );
    REQUIRE( Cy_strnequal( "abc", "abd", 2u ) );
    REQUIRE_FALSE( Cy_strnequal( "abc", "abd", 3u ) );
    REQUIRE( Cy_strniequal( "ABC", "abd", 2u ) );
    REQUIRE_FALSE( Cy_strniequal( "ABC", "abd", 3u ) );
}

TEST_CASE( "Cy_strncpy copies safely and reports required source length", "[CypherCommon][Tier1][String]" )
{
    char pBuffer[8]{};

    REQUIRE( Cy_strncpy( pBuffer, "cypher", sizeof( pBuffer ) ) == 6u );
    REQUIRE( Cy_strequal( pBuffer, "cypher" ) );

    REQUIRE( Cy_strncpy( pBuffer, "engine-runtime", sizeof( pBuffer ) ) == 14u );
    REQUIRE( Cy_strequal( pBuffer, "engine-" ) );

    REQUIRE( Cy_strncpy( pBuffer, nullptr, sizeof( pBuffer ) ) == 0u );
    REQUIRE( Cy_strequal( pBuffer, "" ) );

    REQUIRE( Cy_strncpy( pBuffer, "abc", 0u ) == 3u );
    REQUIRE( Cy_strncpy( nullptr, "abc", 8u ) == 3u );
}

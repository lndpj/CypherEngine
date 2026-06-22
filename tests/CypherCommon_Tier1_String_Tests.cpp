#include "CypherCommon_String.h"

#include <catch2/catch_test_macros.hpp>

using namespace cypher::common;

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

TEST_CASE( "Cy_strncmp respects the maximum character count", "[CypherCommon][Tier1][String]" )
{
    REQUIRE( Cy_strncmp( "abc", "xyz", 0u ) == 0 );
    REQUIRE( Cy_strncmp( "abc", "abd", 2u ) == 0 );
    REQUIRE( Cy_strncmp( "abc", "abd", 3u ) < 0 );
    REQUIRE( Cy_strncmp( "abd", "abc", 3u ) > 0 );
    REQUIRE( Cy_strncmp( "ab", "abc", 2u ) == 0 );
    REQUIRE( Cy_strncmp( "ab", "abc", 3u ) < 0 );
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

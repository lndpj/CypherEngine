#ifndef CYPHER_COMMON_TIER1_STRINGESCAPE_H
#define CYPHER_COMMON_TIER1_STRINGESCAPE_H
#pragma once

/*
================
CypherCommon String Escape

Escaped text declarations for config files, command lines, tools, JSON-like
data and quoted editor strings.
================
*/

#include "CypherCommon_Tier0.h"

namespace cypher::common
{

enum string_escape_flags_t : flags32_t {
    STRING_ESCAPE_FLAG_NONE = 0u,
    STRING_ESCAPE_FLAG_QUOTES = CYPHER_BIT32( 0 ),
    STRING_ESCAPE_FLAG_CONTROL_CHARS = CYPHER_BIT32( 1 ),
    STRING_ESCAPE_FLAG_PATH_SLASHES = CYPHER_BIT32( 2 )
};

usize Cy_strescape( const char *pString, char *pDest, usize cchDest, flags32_t flags );
usize Cy_strunescape( const char *pString, char *pDest, usize cchDest, flags32_t flags );
bool_t Cy_strneedsescape( const char *pString, flags32_t flags );

} // namespace cypher::common

#endif // CYPHER_COMMON_TIER1_STRINGESCAPE_H

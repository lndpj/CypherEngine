#ifndef CYPHER_COMMON_TIER1_STRINGTOKEN_H
#define CYPHER_COMMON_TIER1_STRINGTOKEN_H
#pragma once

/*
================
CypherCommon String Token

Stable hashed string token declarations.
================
*/

#include "CypherCommon_Tier0.h"

namespace cypher::common
{

struct string_token_t {
    u32 hash;
};

string_token_t StringToken_FromString( const char *pString );
bool_t StringToken_IsValid( string_token_t token );
bool_t StringToken_Equals( string_token_t a, string_token_t b );

} // namespace cypher::common

#endif // CYPHER_COMMON_TIER1_STRINGTOKEN_H

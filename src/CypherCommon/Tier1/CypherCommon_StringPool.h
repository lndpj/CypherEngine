#ifndef CYPHER_COMMON_TIER1_STRINGPOOL_H
#define CYPHER_COMMON_TIER1_STRINGPOOL_H
#pragma once

/*
================
CypherCommon String Pool

Interned string pool declarations.
================
*/

#include "CypherCommon_Tier0.h"

namespace cypher::common
{

struct string_pool_t;

bool_t StringPool_Init( string_pool_t *pPool, void *pMemory, usize cbMemory );
void StringPool_Shutdown( string_pool_t *pPool );
const char *StringPool_Intern( string_pool_t *pPool, const char *pString );
bool_t StringPool_Contains( const string_pool_t *pPool, const char *pString );

} // namespace cypher::common

#endif // CYPHER_COMMON_TIER1_STRINGPOOL_H

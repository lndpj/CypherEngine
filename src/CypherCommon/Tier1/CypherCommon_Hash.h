#ifndef CYPHER_COMMON_TIER1_HASH_H
#define CYPHER_COMMON_TIER1_HASH_H
#pragma once

/*
================
CypherCommon Hash

General hash declarations.
================
*/

#include "CypherCommon_Tier0.h"

namespace cypher::common
{

hash32_t Hash32_Data( const void *pData, usize cbData, hash32_t seed );
hash64_t Hash64_Data( const void *pData, usize cbData, hash64_t seed );
hash32_t Hash32_String( const char *pString );
hash64_t Hash64_String( const char *pString );

} // namespace cypher::common

#endif // CYPHER_COMMON_TIER1_HASH_H

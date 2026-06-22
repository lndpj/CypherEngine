#ifndef CYPHER_COMMON_TIER1_HASHFNV_H
#define CYPHER_COMMON_TIER1_HASHFNV_H
#pragma once

/*
================
CypherCommon FNV Hash

FNV hash declarations.
================
*/

#include "CypherCommon_Tier0.h"

namespace cypher::common
{

hash32_t HashFNV1a32_Data( const void *pData, usize cbData );
hash64_t HashFNV1a64_Data( const void *pData, usize cbData );
hash32_t HashFNV1a32_String( const char *pString );
hash64_t HashFNV1a64_String( const char *pString );

} // namespace cypher::common

#endif // CYPHER_COMMON_TIER1_HASHFNV_H

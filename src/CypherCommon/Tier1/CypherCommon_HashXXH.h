#ifndef CYPHER_COMMON_TIER1_HASHXXH_H
#define CYPHER_COMMON_TIER1_HASHXXH_H
#pragma once

/*
================
CypherCommon XX Hash

Fast non-cryptographic hash declarations.
================
*/

#include "CypherCommon_Tier0.h"

namespace cypher::common
{

hash32_t HashXXH32_Data( const void *pData, usize cbData, hash32_t seed );
hash64_t HashXXH64_Data( const void *pData, usize cbData, hash64_t seed );

} // namespace cypher::common

#endif // CYPHER_COMMON_TIER1_HASHXXH_H

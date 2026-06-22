#ifndef CYPHER_COMMON_TIER1_CONTENTHASH_H
#define CYPHER_COMMON_TIER1_CONTENTHASH_H
#pragma once

/*
================
CypherCommon Content Hash

Stable asset/content hash declarations.
================
*/

#include "CypherCommon_Tier0.h"

namespace cypher::common
{

struct content_hash_t {
    u64 high;
    u64 low;
};

content_hash_t ContentHash_Data( const void *pData, usize cbData );
bool_t ContentHash_Equals( content_hash_t a, content_hash_t b );

} // namespace cypher::common

#endif // CYPHER_COMMON_TIER1_CONTENTHASH_H

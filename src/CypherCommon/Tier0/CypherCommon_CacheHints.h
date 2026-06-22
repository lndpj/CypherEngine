#ifndef CYPHER_COMMON_TIER0_CACHEHINTS_H
#define CYPHER_COMMON_TIER0_CACHEHINTS_H
#pragma once

/*
================
CypherCommon Cache Hints

Cache prefetch and cache-line helper declarations.
================
*/

#include "CypherCommon_BaseTypes.h"

namespace cypher::common
{

void Cache_PrefetchRead( const void *pMemory );
void Cache_PrefetchWrite( const void *pMemory );
usize Cache_GetLineSize();

} // namespace cypher::common

#endif // CYPHER_COMMON_TIER0_CACHEHINTS_H

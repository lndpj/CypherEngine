#ifndef CYPHER_COMMON_TIER1_MEMORYPOOL_H
#define CYPHER_COMMON_TIER1_MEMORYPOOL_H
#pragma once

/*
================
CypherCommon Memory Pool

Fixed-size object pool declarations.
================
*/

#include "CypherCommon_Tier0.h"

namespace cypher::common
{

struct memory_pool_t;

bool_t MemoryPool_Init( memory_pool_t *pPool, void *pMemory, usize cbMemory, usize cbElement );
void *MemoryPool_Alloc( memory_pool_t *pPool );
void MemoryPool_Free( memory_pool_t *pPool, void *pElement );

} // namespace cypher::common

#endif // CYPHER_COMMON_TIER1_MEMORYPOOL_H

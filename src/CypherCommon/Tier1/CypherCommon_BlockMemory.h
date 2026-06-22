#ifndef CYPHER_COMMON_TIER1_BLOCKMEMORY_H
#define CYPHER_COMMON_TIER1_BLOCKMEMORY_H
#pragma once

/*
================
CypherCommon Block Memory

Block-based memory declarations.
================
*/

#include "CypherCommon_Tier0.h"

namespace cypher::common
{

struct block_memory_t;

bool_t BlockMemory_Init( block_memory_t *pMemory, usize cbBlockSize, usize block_count );
void BlockMemory_Shutdown( block_memory_t *pMemory );
void *BlockMemory_AllocBlock( block_memory_t *pMemory );
void BlockMemory_FreeBlock( block_memory_t *pMemory, void *pBlock );

} // namespace cypher::common

#endif // CYPHER_COMMON_TIER1_BLOCKMEMORY_H

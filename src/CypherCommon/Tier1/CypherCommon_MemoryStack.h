#ifndef CYPHER_COMMON_TIER1_MEMORYSTACK_H
#define CYPHER_COMMON_TIER1_MEMORYSTACK_H
#pragma once

/*
================
CypherCommon Memory Stack

Linear stack allocator declarations.
================
*/

#include "CypherCommon_Tier0.h"

namespace cypher::common
{

struct memory_stack_t;

bool_t MemoryStack_Init( memory_stack_t *pStack, void *pMemory, usize cbMemory );
void *MemoryStack_Alloc( memory_stack_t *pStack, usize cbSize, usize alignment );
void MemoryStack_Reset( memory_stack_t *pStack );

} // namespace cypher::common

#endif // CYPHER_COMMON_TIER1_MEMORYSTACK_H

#ifndef CYPHER_COMMON_TIER1_FIXEDMEMORY_H
#define CYPHER_COMMON_TIER1_FIXEDMEMORY_H
#pragma once

/*
================
CypherCommon Fixed Memory

Fixed buffer memory declarations.
================
*/

#include "CypherCommon_Tier0.h"

namespace cypher::common
{

struct fixed_memory_t {
    void *pMemory;
    usize cbSize;
};

void FixedMemory_Init( fixed_memory_t *pMemory, void *pData, usize cbSize );
bool_t FixedMemory_Contains( const fixed_memory_t *pMemory, const void *pAddress );

} // namespace cypher::common

#endif // CYPHER_COMMON_TIER1_FIXEDMEMORY_H

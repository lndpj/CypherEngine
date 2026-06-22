#ifndef CYPHER_COMMON_TIER0_PLATFORMMEMORY_H
#define CYPHER_COMMON_TIER0_PLATFORMMEMORY_H
#pragma once

/*
================
CypherCommon Platform Memory

Operating-system memory declarations.
================
*/

#include "CypherCommon_BaseTypes.h"

namespace cypher::common
{

struct platform_memory_info_t {
    usize page_size;
    usize allocation_granularity;
    u64 total_physical_memory;
    u64 available_physical_memory;
};

platform_memory_info_t PlatformMemory_GetInfo();
void *PlatformMemory_Reserve( usize cbSize );
bool_t PlatformMemory_Commit( void *pMemory, usize cbSize );
void PlatformMemory_Decommit( void *pMemory, usize cbSize );
void PlatformMemory_Release( void *pMemory, usize cbSize );

} // namespace cypher::common

#endif // CYPHER_COMMON_TIER0_PLATFORMMEMORY_H

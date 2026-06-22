#ifndef CYPHER_COMMON_TIER0_MEMORYTRACKER_H
#define CYPHER_COMMON_TIER0_MEMORYTRACKER_H
#pragma once

/*
================
CypherCommon Memory Tracker

Allocation tracking declarations.
================
*/

#include "CypherCommon_BaseTypes.h"

namespace cypher::common
{

struct memory_allocation_record_t {
    void *pMemory;
    usize cbSize;
    usize alignment;
    const char *pTag;
    const char *pFile;
    i32 line;
};

void MemoryTracker_RecordAlloc( const memory_allocation_record_t &record );
void MemoryTracker_RecordFree( void *pMemory );
usize MemoryTracker_GetLiveAllocationCount();
usize MemoryTracker_GetLiveByteCount();

} // namespace cypher::common

#endif // CYPHER_COMMON_TIER0_MEMORYTRACKER_H

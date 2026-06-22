#ifndef CYPHER_COMMON_TIER0_PAGEALLOCATOR_H
#define CYPHER_COMMON_TIER0_PAGEALLOCATOR_H
#pragma once

/*
================
CypherCommon Page Allocator

Page-granular allocator declarations.
================
*/

#include "CypherCommon_BaseTypes.h"

namespace cypher::common
{

struct page_allocator_t {
    void *pReservedBase;
    usize cbReserved;
    usize cbCommitted;
    usize page_size;
};

bool_t PageAllocator_Init( page_allocator_t *pAllocator, usize cbReserve );
void PageAllocator_Shutdown( page_allocator_t *pAllocator );
void *PageAllocator_Commit( page_allocator_t *pAllocator, usize cbSize );
void PageAllocator_Reset( page_allocator_t *pAllocator );

} // namespace cypher::common

#endif // CYPHER_COMMON_TIER0_PAGEALLOCATOR_H

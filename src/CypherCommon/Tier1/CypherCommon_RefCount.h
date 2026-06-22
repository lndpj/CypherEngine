#ifndef CYPHER_COMMON_TIER1_REFCOUNT_H
#define CYPHER_COMMON_TIER1_REFCOUNT_H
#pragma once

/*
================
CypherCommon Ref Count

Reference counting declarations.
================
*/

#include "CypherCommon_Tier0.h"

namespace cypher::common
{

struct ref_count_t {
    u32 count;
};

void RefCount_Init( ref_count_t *pRefCount, u32 initial_count );
u32 RefCount_AddRef( ref_count_t *pRefCount );
u32 RefCount_Release( ref_count_t *pRefCount );

} // namespace cypher::common

#endif // CYPHER_COMMON_TIER1_REFCOUNT_H

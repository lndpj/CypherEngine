#ifndef CYPHER_COMMON_TIER1_OBJECTPOOL_H
#define CYPHER_COMMON_TIER1_OBJECTPOOL_H
#pragma once

/*
================
CypherCommon Object Pool

Typed object pool declarations.
================
*/

#include "CypherCommon_Tier0.h"

namespace cypher::common
{

template <typename type_t>
struct object_pool_t;

template <typename type_t>
type_t *ObjectPool_Alloc( object_pool_t<type_t> *pPool );

template <typename type_t>
void ObjectPool_Free( object_pool_t<type_t> *pPool, type_t *pObject );

} // namespace cypher::common

#endif // CYPHER_COMMON_TIER1_OBJECTPOOL_H

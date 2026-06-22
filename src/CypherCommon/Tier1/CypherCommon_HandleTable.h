#ifndef CYPHER_COMMON_TIER1_HANDLETABLE_H
#define CYPHER_COMMON_TIER1_HANDLETABLE_H
#pragma once

/*
================
CypherCommon Handle Table

Generational handle table declarations.
================
*/

#include "CypherCommon_Tier0.h"

namespace cypher::common
{

template <typename type_t>
struct handle_table_t;

template <typename type_t>
handle_t HandleTable_Add( handle_table_t<type_t> *pTable, const type_t &value );

template <typename type_t>
type_t *HandleTable_Get( handle_table_t<type_t> *pTable, handle_t handle );

} // namespace cypher::common

#endif // CYPHER_COMMON_TIER1_HANDLETABLE_H

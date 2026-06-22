#ifndef CYPHER_COMMON_TIER1_FIXEDARRAY_H
#define CYPHER_COMMON_TIER1_FIXEDARRAY_H
#pragma once

/*
================
CypherCommon Fixed Array

Fixed-capacity array declarations.
================
*/

#include "CypherCommon_Tier0.h"

namespace cypher::common
{

template <typename type_t, usize capacity>
struct fixed_array_t;

template <typename type_t, usize capacity>
bool_t FixedArray_Push( fixed_array_t<type_t, capacity> *pArray, const type_t &value );

} // namespace cypher::common

#endif // CYPHER_COMMON_TIER1_FIXEDARRAY_H

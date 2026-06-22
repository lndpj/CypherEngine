#ifndef CYPHER_COMMON_TIER1_ARRAY_H
#define CYPHER_COMMON_TIER1_ARRAY_H
#pragma once

/*
================
CypherCommon Array

Owning dynamic array declarations.
================
*/

#include "CypherCommon_Tier0.h"

namespace cypher::common
{

template <typename type_t>
struct array_t;

template <typename type_t>
bool_t Array_Init( array_t<type_t> *pArray, usize capacity );

template <typename type_t>
void Array_Shutdown( array_t<type_t> *pArray );

} // namespace cypher::common

#endif // CYPHER_COMMON_TIER1_ARRAY_H

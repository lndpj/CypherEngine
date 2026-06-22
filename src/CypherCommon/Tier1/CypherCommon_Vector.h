#ifndef CYPHER_COMMON_TIER1_VECTOR_H
#define CYPHER_COMMON_TIER1_VECTOR_H
#pragma once

/*
================
CypherCommon Vector

Source-style growable vector declarations.
================
*/

#include "CypherCommon_Tier0.h"

namespace cypher::common
{

template <typename type_t>
struct vector_t;

template <typename type_t>
bool_t Vector_Init( vector_t<type_t> *pVector, usize initial_capacity );

template <typename type_t>
void Vector_Shutdown( vector_t<type_t> *pVector );

template <typename type_t>
bool_t Vector_PushBack( vector_t<type_t> *pVector, const type_t &value );

} // namespace cypher::common

#endif // CYPHER_COMMON_TIER1_VECTOR_H

#ifndef CYPHER_COMMON_TIER1_STACK_H
#define CYPHER_COMMON_TIER1_STACK_H
#pragma once

/*
================
CypherCommon Stack

LIFO container declarations.
================
*/

#include "CypherCommon_Tier0.h"

namespace cypher::common
{

template <typename type_t>
struct stack_t;

template <typename type_t>
bool_t Stack_Push( stack_t<type_t> *pStack, const type_t &value );

template <typename type_t>
bool_t Stack_Pop( stack_t<type_t> *pStack, type_t *pOutValue );

} // namespace cypher::common

#endif // CYPHER_COMMON_TIER1_STACK_H

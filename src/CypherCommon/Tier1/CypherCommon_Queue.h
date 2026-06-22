#ifndef CYPHER_COMMON_TIER1_QUEUE_H
#define CYPHER_COMMON_TIER1_QUEUE_H
#pragma once

/*
================
CypherCommon Queue

FIFO container declarations.
================
*/

#include "CypherCommon_Tier0.h"

namespace cypher::common
{

template <typename type_t>
struct queue_t;

template <typename type_t>
bool_t Queue_Push( queue_t<type_t> *pQueue, const type_t &value );

template <typename type_t>
bool_t Queue_Pop( queue_t<type_t> *pQueue, type_t *pOutValue );

} // namespace cypher::common

#endif // CYPHER_COMMON_TIER1_QUEUE_H

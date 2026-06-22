#ifndef CYPHER_COMMON_TIER1_CALLQUEUE_H
#define CYPHER_COMMON_TIER1_CALLQUEUE_H
#pragma once

/*
================
CypherCommon Call Queue

Deferred call queue declarations.
================
*/

#include "CypherCommon_Tier0.h"

namespace cypher::common
{

using call_queue_proc_t = void ( * )( void *pUserData );

struct call_queue_t;

bool_t CallQueue_Push( call_queue_t *pQueue, call_queue_proc_t proc, void *pUserData );
void CallQueue_Drain( call_queue_t *pQueue );

} // namespace cypher::common

#endif // CYPHER_COMMON_TIER1_CALLQUEUE_H

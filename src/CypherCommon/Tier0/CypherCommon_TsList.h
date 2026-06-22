#ifndef CYPHER_COMMON_TIER0_TSLIST_H
#define CYPHER_COMMON_TIER0_TSLIST_H
#pragma once

/*
================
CypherCommon TS List

Thread-safe intrusive single-list declarations.
================
*/

#include "CypherCommon_BaseTypes.h"

namespace cypher::common
{

struct tslist_node_t {
    tslist_node_t *pNext;
};

struct tslist_t;

void TsList_Init( tslist_t *pList );
void TsList_Push( tslist_t *pList, tslist_node_t *pNode );
tslist_node_t *TsList_Pop( tslist_t *pList );

} // namespace cypher::common

#endif // CYPHER_COMMON_TIER0_TSLIST_H

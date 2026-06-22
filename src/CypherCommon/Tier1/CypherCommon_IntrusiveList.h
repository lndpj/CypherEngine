#ifndef CYPHER_COMMON_TIER1_INTRUSIVELIST_H
#define CYPHER_COMMON_TIER1_INTRUSIVELIST_H
#pragma once

/*
================
CypherCommon Intrusive List

Intrusive list declarations.
================
*/

namespace cypher::common
{

struct intrusive_list_node_t {
    intrusive_list_node_t *pPrev;
    intrusive_list_node_t *pNext;
};

struct intrusive_list_t;

void IntrusiveList_Init( intrusive_list_t *pList );
void IntrusiveList_PushBack( intrusive_list_t *pList, intrusive_list_node_t *pNode );
void IntrusiveList_Remove( intrusive_list_t *pList, intrusive_list_node_t *pNode );

} // namespace cypher::common

#endif // CYPHER_COMMON_TIER1_INTRUSIVELIST_H

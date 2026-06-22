#ifndef CYPHER_COMMON_TIER1_SORT_H
#define CYPHER_COMMON_TIER1_SORT_H
#pragma once

/*
================
CypherCommon Sort

Sorting declarations.
================
*/

#include "CypherCommon_Tier0.h"

namespace cypher::common
{

using sort_compare_t = i32 ( * )( const void *pA, const void *pB, void *pUserData );

void Sort_Quick( void *pData, usize count, usize cbElement, sort_compare_t compare, void *pUserData );
void Sort_Insertion( void *pData, usize count, usize cbElement, sort_compare_t compare, void *pUserData );

} // namespace cypher::common

#endif // CYPHER_COMMON_TIER1_SORT_H

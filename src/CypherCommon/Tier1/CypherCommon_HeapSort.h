#ifndef CYPHER_COMMON_TIER1_HEAPSORT_H
#define CYPHER_COMMON_TIER1_HEAPSORT_H
#pragma once

/*
================
CypherCommon Heap Sort

Heap sort declarations.
================
*/

#include "CypherCommon_Sort.h"

namespace cypher::common
{

void HeapSort_Sort( void *pData, usize count, usize cbElement, sort_compare_t compare, void *pUserData );

} // namespace cypher::common

#endif // CYPHER_COMMON_TIER1_HEAPSORT_H

#ifndef CYPHER_COMMON_TIER1_SEARCH_H
#define CYPHER_COMMON_TIER1_SEARCH_H
#pragma once

/*
================
CypherCommon Search

Search helper declarations.
================
*/

#include "CypherCommon_Tier0.h"
#include "CypherCommon_Sort.h"

namespace cypher::common
{

void *Search_Linear( void *pData, usize count, usize cbElement, const void *pKey, sort_compare_t compare, void *pUserData );
void *Search_Binary( void *pData, usize count, usize cbElement, const void *pKey, sort_compare_t compare, void *pUserData );

} // namespace cypher::common

#endif // CYPHER_COMMON_TIER1_SEARCH_H

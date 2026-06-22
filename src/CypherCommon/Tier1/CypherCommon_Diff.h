#ifndef CYPHER_COMMON_TIER1_DIFF_H
#define CYPHER_COMMON_TIER1_DIFF_H
#pragma once

/*
================
CypherCommon Diff

Binary/text diff declaration surface.
================
*/

#include "CypherCommon_Tier0.h"

namespace cypher::common
{

struct diff_result_t;

bool_t Diff_Binary( const void *pOldData, usize cbOldData, const void *pNewData, usize cbNewData, diff_result_t *pOutDiff );
bool_t Diff_Apply( const void *pOldData, usize cbOldData, const diff_result_t *pDiff, void *pDest, usize cbDest );

} // namespace cypher::common

#endif // CYPHER_COMMON_TIER1_DIFF_H

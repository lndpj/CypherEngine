#ifndef CYPHER_COMMON_TIER1_ARRAYVIEW_H
#define CYPHER_COMMON_TIER1_ARRAYVIEW_H
#pragma once

/*
================
CypherCommon Array View

Read-only array view declarations.
================
*/

#include "CypherCommon_Tier0.h"

namespace cypher::common
{

template <typename type_t>
struct array_view_t {
    const type_t *pData;
    usize count;
};

template <typename type_t>
array_view_t<type_t> ArrayView_FromPointerCount( const type_t *pData, usize count );

} // namespace cypher::common

#endif // CYPHER_COMMON_TIER1_ARRAYVIEW_H

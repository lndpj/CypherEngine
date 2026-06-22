#ifndef CYPHER_COMMON_TIER1_SPAN_H
#define CYPHER_COMMON_TIER1_SPAN_H
#pragma once

/*
================
CypherCommon Span

Non-owning contiguous array view declarations.
================
*/

#include "CypherCommon_Tier0.h"

namespace cypher::common
{

template <typename type_t>
struct span_t {
    type_t *pData;
    usize count;
};

template <typename type_t>
span_t<type_t> Span_FromPointerCount( type_t *pData, usize count );

template <typename type_t>
bool_t Span_IsEmpty( span_t<type_t> span );

} // namespace cypher::common

#endif // CYPHER_COMMON_TIER1_SPAN_H

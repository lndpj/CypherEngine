#ifndef CYPHER_COMMON_TIER1_SMALLVECTOR_H
#define CYPHER_COMMON_TIER1_SMALLVECTOR_H
#pragma once

/*
================
CypherCommon Small Vector

Small-buffer vector declarations.
================
*/

#include "CypherCommon_Tier0.h"

namespace cypher::common
{

template <typename type_t, usize inline_capacity>
struct small_vector_t;

} // namespace cypher::common

#endif // CYPHER_COMMON_TIER1_SMALLVECTOR_H

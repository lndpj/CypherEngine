#ifndef CYPHER_COMMON_TIER1_RANGE_H
#define CYPHER_COMMON_TIER1_RANGE_H
#pragma once

/*
================
CypherCommon Range

Index/count and min/max range declarations.
================
*/

#include "CypherCommon_Tier0.h"

namespace cypher::common
{

struct index_range_t {
    usize first;
    usize count;
};

template <typename type_t>
struct value_range_t {
    type_t min_value;
    type_t max_value;
};

} // namespace cypher::common

#endif // CYPHER_COMMON_TIER1_RANGE_H

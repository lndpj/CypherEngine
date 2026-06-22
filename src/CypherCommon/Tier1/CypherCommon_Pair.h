#ifndef CYPHER_COMMON_TIER1_PAIR_H
#define CYPHER_COMMON_TIER1_PAIR_H
#pragma once

/*
================
CypherCommon Pair

Small two-value aggregate declarations.
================
*/

namespace cypher::common
{

template <typename first_t, typename second_t>
struct pair_t {
    first_t first;
    second_t second;
};

} // namespace cypher::common

#endif // CYPHER_COMMON_TIER1_PAIR_H

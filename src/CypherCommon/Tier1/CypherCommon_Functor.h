#ifndef CYPHER_COMMON_TIER1_FUNCTOR_H
#define CYPHER_COMMON_TIER1_FUNCTOR_H
#pragma once

/*
================
CypherCommon Functor

Functor utility declarations.
================
*/

namespace cypher::common
{

template <typename type_t>
struct less_t;

template <typename type_t>
struct equal_t;

template <typename type_t>
struct hash_functor_t;

} // namespace cypher::common

#endif // CYPHER_COMMON_TIER1_FUNCTOR_H

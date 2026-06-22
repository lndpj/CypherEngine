#ifndef CYPHER_COMMON_TIER1_FIXEDSTRING_H
#define CYPHER_COMMON_TIER1_FIXEDSTRING_H
#pragma once

/*
================
CypherCommon Fixed String

Fixed-capacity stack string declarations.
================
*/

#include "CypherCommon_Tier0.h"

namespace cypher::common
{

template <usize cchCapacity>
struct fixed_string_t;

template <usize cchCapacity>
usize FixedString_Length( const fixed_string_t<cchCapacity> &string );

template <usize cchCapacity>
usize FixedString_Copy( fixed_string_t<cchCapacity> *pString, const char *pSrc );

template <usize cchCapacity>
usize FixedString_Append( fixed_string_t<cchCapacity> *pString, const char *pSrc );

} // namespace cypher::common

#endif // CYPHER_COMMON_TIER1_FIXEDSTRING_H

#ifndef CYPHER_COMMON_TIER0_PERFORMANCECOUNTER_H
#define CYPHER_COMMON_TIER0_PERFORMANCECOUNTER_H
#pragma once

/*
================
CypherCommon Performance Counter

High-resolution counter declarations.
================
*/

#include "CypherCommon_BaseTypes.h"

namespace cypher::common
{

using performance_counter_t = u64;

performance_counter_t PerformanceCounter_Now();
u64 PerformanceCounter_Frequency();
f64 PerformanceCounter_ToSeconds( performance_counter_t ticks );

} // namespace cypher::common

#endif // CYPHER_COMMON_TIER0_PERFORMANCECOUNTER_H

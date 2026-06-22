#ifndef CYPHER_COMMON_TIER1_RELIABLETIMER_H
#define CYPHER_COMMON_TIER1_RELIABLETIMER_H
#pragma once

/*
================
CypherCommon Reliable Timer

Stable interval timer declarations.
================
*/

#include "CypherCommon_Tier0.h"

namespace cypher::common
{

struct reliable_timer_t {
    u64 start_ticks;
    u64 interval_ticks;
};

void ReliableTimer_Start( reliable_timer_t *pTimer, f64 interval_seconds );
bool_t ReliableTimer_HasElapsed( const reliable_timer_t *pTimer );
void ReliableTimer_Restart( reliable_timer_t *pTimer );

} // namespace cypher::common

#endif // CYPHER_COMMON_TIER1_RELIABLETIMER_H

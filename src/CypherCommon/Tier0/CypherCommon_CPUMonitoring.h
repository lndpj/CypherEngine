#ifndef CYPHER_COMMON_TIER0_CPUMONITORING_H
#define CYPHER_COMMON_TIER0_CPUMONITORING_H
#pragma once

/*
================
CypherCommon CPU Monitoring

CPU monitoring sample declarations.
================
*/

#include "CypherCommon_BaseTypes.h"

namespace cypher::common
{

struct cpu_monitor_sample_t {
    f32 total_usage;
    f32 process_usage;
    u32 logical_thread_count;
};

bool_t CPUMonitoring_Sample( cpu_monitor_sample_t *pOutSample );

} // namespace cypher::common

#endif // CYPHER_COMMON_TIER0_CPUMONITORING_H

#ifndef CYPHER_COMMON_TIER1_PROCESSORDETECT_H
#define CYPHER_COMMON_TIER1_PROCESSORDETECT_H
#pragma once

/*
================
CypherCommon Processor Detect

Higher-level processor detection declarations.
================
*/

#include "CypherCommon_Tier0.h"

namespace cypher::common
{

struct processor_info_t {
    char name[128];
    u32 logical_thread_count;
    u32 physical_core_count;
};

bool_t ProcessorDetect_GetInfo( processor_info_t *pOutInfo );

} // namespace cypher::common

#endif // CYPHER_COMMON_TIER1_PROCESSORDETECT_H

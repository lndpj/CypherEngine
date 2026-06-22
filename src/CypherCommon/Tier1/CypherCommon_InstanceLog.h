#ifndef CYPHER_COMMON_TIER1_INSTANCELOG_H
#define CYPHER_COMMON_TIER1_INSTANCELOG_H
#pragma once

/*
================
CypherCommon Instance Log

Per-instance event log declarations.
================
*/

#include "CypherCommon_Tier0.h"

namespace cypher::common
{

struct instance_log_t;

void InstanceLog_Add( instance_log_t *pLog, const char *pMessage );
void InstanceLog_Clear( instance_log_t *pLog );

} // namespace cypher::common

#endif // CYPHER_COMMON_TIER1_INSTANCELOG_H

#ifndef CYPHER_COMMON_TIER0_LOGTOGGLE_H
#define CYPHER_COMMON_TIER0_LOGTOGGLE_H
#pragma once

/*
================
CypherCommon Log Toggle

Compile-time log category toggle declarations.
================
*/

#include "CypherCommon_BaseTypes.h"

namespace cypher::common
{

using log_category_mask_t = u64;

void LogToggle_Enable( log_category_mask_t category_mask );
void LogToggle_Disable( log_category_mask_t category_mask );
bool_t LogToggle_IsEnabled( log_category_mask_t category_mask );

} // namespace cypher::common

#endif // CYPHER_COMMON_TIER0_LOGTOGGLE_H

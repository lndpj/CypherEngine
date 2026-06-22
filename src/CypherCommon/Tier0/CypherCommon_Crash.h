#ifndef CYPHER_COMMON_TIER0_CRASH_H
#define CYPHER_COMMON_TIER0_CRASH_H
#pragma once

/*
================
CypherCommon Crash

Low-level crash reporting declarations.
================
*/

#include "CypherCommon_BaseTypes.h"

namespace cypher::common
{

using crash_handler_t = void ( * )( const char *pReason, const char *pFile, i32 line );

void Crash_SetHandler( crash_handler_t handler );
void Crash_ReportFatal( const char *pReason, const char *pFile, i32 line );
void Crash_Trigger( const char *pReason );

} // namespace cypher::common

#endif // CYPHER_COMMON_TIER0_CRASH_H

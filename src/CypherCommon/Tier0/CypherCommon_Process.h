#ifndef CYPHER_COMMON_TIER0_PROCESS_H
#define CYPHER_COMMON_TIER0_PROCESS_H
#pragma once

/*
================
CypherCommon Process

Process identity and process utility declarations.
================
*/

#include "CypherCommon_BaseTypes.h"

namespace cypher::common
{

using process_id_t = u64;

process_id_t Process_GetCurrentId();
const char *Process_GetExecutablePath();
void Process_Exit( i32 exit_code );

} // namespace cypher::common

#endif // CYPHER_COMMON_TIER0_PROCESS_H

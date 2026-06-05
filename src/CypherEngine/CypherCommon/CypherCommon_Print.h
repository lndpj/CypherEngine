#pragma once

#include "CypherEngine/CypherCommon/CypherCommon_Error.h"

#include <cstdarg>     // va_list for variadic print forwarding.

namespace cypher::engine::common
{

constexpr usize COM_MSG_MAX = 2048u;

/*
================
Common Print API

Small printf-style output layer used before and beside the structured logger.
================
*/
void CypherCommon_Printf( const char *message, ... );

void CypherCommon_DPrintf( const char *message, ... );

void CypherCommon_VPrintf( const char *message, va_list args );

void CypherCommon_Errorf( const error_t error, const char *message, ... );

void CypherCommon_VErrorf( const error_t error, const char *message, va_list args );

}

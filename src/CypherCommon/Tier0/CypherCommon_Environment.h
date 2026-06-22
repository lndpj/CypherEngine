#ifndef CYPHER_COMMON_TIER0_ENVIRONMENT_H
#define CYPHER_COMMON_TIER0_ENVIRONMENT_H
#pragma once

/*
================
CypherCommon Environment

Environment variable declarations.
================
*/

#include "CypherCommon_BaseTypes.h"

namespace cypher::common
{

usize Environment_Get( const char *pName, char *pDest, usize cchDest );
bool_t Environment_Set( const char *pName, const char *pValue );
bool_t Environment_Has( const char *pName );

} // namespace cypher::common

#endif // CYPHER_COMMON_TIER0_ENVIRONMENT_H

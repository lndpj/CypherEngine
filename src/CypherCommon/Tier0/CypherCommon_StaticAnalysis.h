#ifndef CYPHER_COMMON_TIER0_STATICANALYSIS_H
#define CYPHER_COMMON_TIER0_STATICANALYSIS_H
#pragma once

/*
================
CypherCommon Static Analysis

Static-analysis and code-audit macro surface.
================
*/

#include "CypherCommon_Debug.h"
#include "CypherCommon_Defines.h"

#define CY_ANALYSIS_ASSUME( expression )    do { CYPHER_UNUSED( expression ); } while ( 0 )
#define CY_ANALYSIS_SUPPRESS( id )
#define CY_ANALYSIS_UNREACHABLE()          CYPHER_TRAP()

#endif // CYPHER_COMMON_TIER0_STATICANALYSIS_H

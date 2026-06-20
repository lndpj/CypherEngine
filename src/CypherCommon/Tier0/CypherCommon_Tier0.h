#ifndef CYPHER_COMMON_TIER0_H
#define CYPHER_COMMON_TIER0_H
#pragma once

/*
================
CypherCommon Tier0

Lowest-level foundation shared by engine runtime, game code, tools and editor.
This header is allowed to know the target platform and primitive scalar types,
but it must not depend on any CypherEngine subsystem.
================
*/

#include "CypherCommon_Platform.h"
#include "CypherCommon_BaseTypes.h"
#include "CypherCommon_Defines.h"

#endif // CYPHER_COMMON_TIER0_H

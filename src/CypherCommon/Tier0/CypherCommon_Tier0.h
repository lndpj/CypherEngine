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
#include "CypherCommon_Debug.h"
#include "CypherCommon_Assert.h"
#include "CypherCommon_MemoryOps.h"
#include "CypherCommon_Align.h"
#include "CypherCommon_Bits.h"
#include "CypherCommon_Endian.h"
#include "CypherCommon_TypeTraits.h"
#include "CypherCommon_Thread.h"
#include "CypherCommon_Timer.h"
#include "CypherCommon_StackTrace.h"
#include "CypherCommon_SystemInfo.h"

#endif // CYPHER_COMMON_TIER0_H

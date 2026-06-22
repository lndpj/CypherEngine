#ifndef CYPHER_COMMON_TIER0_BUILDCONFIG_H
#define CYPHER_COMMON_TIER0_BUILDCONFIG_H
#pragma once

/*
================
CypherCommon Build Config

Build configuration declarations shared by every module.
================
*/

#include "CypherCommon_BaseTypes.h"

namespace cypher::common
{

enum class build_config_t : u32 {
    Unknown = 0u,
    Debug,
    Release,
    Retail
};

build_config_t BuildConfig_GetCurrent();
const char *BuildConfig_GetName( build_config_t config );
bool_t BuildConfig_IsDebug();
bool_t BuildConfig_IsRelease();

} // namespace cypher::common

#endif // CYPHER_COMMON_TIER0_BUILDCONFIG_H

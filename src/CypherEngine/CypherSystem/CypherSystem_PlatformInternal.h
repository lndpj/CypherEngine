#pragma once

#include "CypherEngine/CypherSystem/CypherSystem_Platform.h"
#include "CypherEngine/CypherSystem/CypherSystem_Error.h"

namespace cypher::engine::sys {

/*
================
Platform Internal API

Implemented separately by macOS, Linux and Win32 translation units.
================
*/
cypher_system_error_code_t CypherSystem_PlatformBuildPaths( const cypher_system_init_info_t &info_init, cypher_system_paths_t &out_paths );

void CypherSystem_PlatformSleepMilliseconds(const common::u64 milliseconds );

} // namespace cypher::engine::sys

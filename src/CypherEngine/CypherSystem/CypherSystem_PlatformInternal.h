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
sys_error_t CypherSystem_PlatformBuildPaths( const init_info_t &info_init, paths_t &out_paths );

void CypherSystem_PlatformSleepMilliseconds(const common::u64 milliseconds );

common::usize CypherSystem_PlatformVirtualPageSize();

void *CypherSystem_PlatformVirtualReserve( common::usize size );

bool CypherSystem_PlatformVirtualCommit( void *memory, common::usize size );

bool CypherSystem_PlatformVirtualDecommit( void *memory, common::usize size );

bool CypherSystem_PlatformVirtualRelease( void *memory, common::usize size );

} // namespace cypher::engine::sys

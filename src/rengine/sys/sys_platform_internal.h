#pragma once

#include "rengine/sys/sys_platform.h"
#include "rengine/sys/sys_error.h"

namespace reap::rengine::sys {

/*
================
Platform Internal API

Implemented separately by macOS, Linux and Win32 translation units.
================
*/
sys_error_code_t Sys_PlatformBuildPaths( const sys_init_info_t &info_init, sys_paths_t &out_paths );

void Sys_PlatformSleepMilliseconds(const rcommon::u64 milliseconds );

} // namespace reap::rengine::sys

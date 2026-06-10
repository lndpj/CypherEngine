/*======================================================================
   File: CypherSystem_Platform.cpp
   Project: CypherEngine
   Author: ksiric <email@example.com>
   Created: 2026-04-20 17:42:16
   Last Modified by: ksiric
   Last Modified: 2026-06-10 10:33:03
   ---------------------------------------------------------------------
   Description:
       
   ---------------------------------------------------------------------
   License: 
   Company: 
   Version: 0.1.0
 ======================================================================
                                                                       */
#include "CypherEngine/CypherSystem/CypherSystem_Platform.h"
#include "CypherEngine/CypherSystem/CypherSystem_PlatformInternal.h"

#include <cstring>     // strncpy and path string helpers.
#include <chrono>      // steady_clock timing.

namespace cypher::engine::sys
{

runtime_state_t g_sys_runtime_state;

namespace {

/*
================
CypherSystem_AlignVirtualMemorySize

Rounds virtual-memory operation sizes up to the platform page size.
================
*/
common::usize CypherSystem_AlignVirtualMemorySize( const common::usize size )
{
    if ( size == 0u ) {
        return 0u;
    }

    const common::usize page_size = CypherSystem_PlatformVirtualPageSize();
    if ( page_size == 0u ) {
        return size;
    }

    const common::usize remainder = size % page_size;
    if ( remainder == 0u ) {
        return size;
    }

    return size + ( page_size - remainder );
}

}       // namespace

/*
================
CypherSystem_PlatformType
================
*/
platform_t CypherSystem_PlatformType() {
#   if      CYPHER_PLATFORM_WINDOWS
                return platform_t::WINDOWS;
#   elif    CYPHER_PLATFORM_MACOS
                return platform_t::MACOSX;
#   elif     CYPHER_PLATFORM_LINUX
                return platform_t::LINUX;
#   else
                return platform_t::UNKNOWN;
#   endif
}    

/*
================
CypherSystem_CompilerType
================
*/
compiler_t CypherSystem_CompilerType() {
#   if      CYPHER_COMPILER_MSVC
                return compiler_t::MSVC;
#   elif    CYPHER_COMPILER_CLANG
                return compiler_t::CLANG;
#   elif    CYPHER_COMPILER_GCC
                return compiler_t::GCC;
#   else
                return compiler_t::UNKNOWN;
#   endif
}

/*
================
CypherSystem_Init

Copies startup info and builds platform paths.
================
*/
error_code_t CypherSystem_Init( const init_info_t &info_init ) {
    if ( g_sys_runtime_state.initialized ) {
        return error_code_t::ERR_IS_INIT;
    }
    if ( info_init.app_name == nullptr || info_init.app_name[0] == '\0' ) {
        return error_code_t::ERR_INVALID_ARGUMENT;
    }
    if ( info_init.organization_name == nullptr || info_init.organization_name[0] == '\0' ) {
        return error_code_t::ERR_INVALID_ARGUMENT;
    }
    g_sys_runtime_state = {};
    
    std::strncpy(
                 g_sys_runtime_state.app_name,
                 info_init.app_name,
                 sizeof( g_sys_runtime_state.app_name ) - 1u 
    );
    
    std::strncpy(
                 g_sys_runtime_state.organization_name,
                 info_init.organization_name,
                 sizeof( g_sys_runtime_state.organization_name ) - 1u 
    );
    
    g_sys_runtime_state.argc = info_init.argc;
    g_sys_runtime_state.argv = info_init.argv;
    
    error_code_t paths_result = CypherSystem_PlatformBuildPaths( info_init, g_sys_runtime_state.sys_paths );
    
    if ( paths_result != error_code_t::OK ) {
        g_sys_runtime_state = {};
        return paths_result;
    }
    
    g_sys_runtime_state.initialized = true;
    
    return error_code_t::OK;
}

/*
================
CypherSystem_Shutdown
================
*/
error_code_t CypherSystem_Shutdown() {
    if ( !g_sys_runtime_state.initialized ) {
        return error_code_t::ERR_NOT_INIT;
    }
    
    g_sys_runtime_state = {};
    
    g_sys_runtime_state.initialized = false;
    
    return error_code_t::OK;
}

/*
================
CypherSystem_IsInitialized
================
*/
bool CypherSystem_IsInitialized() {
    return g_sys_runtime_state.initialized;
}

/*
================
CypherSystem_PlatformName
================
*/
const char *CypherSystem_PlatformName( platform_t type ) {
    switch( type ) {
        case platform_t::WINDOWS: return "Windows";
    case platform_t::LINUX: return "Linux";
    case platform_t::MACOSX: return "MacOS";
        default: return "Unknown";
    }
}   

/*
================
CypherSystem_CompilerName
================
*/
const char *CypherSystem_CompilerName( compiler_t type ) {
    switch( type ) {
        case compiler_t::CLANG: return "Clang";
        case compiler_t::GCC: return "GCC";
        case compiler_t::MSVC: return "MSVC";
        default: return "Unknown";
    }
}

/*
================
CypherSystem_PathBasename
================
*/
const char *CypherSystem_PathBasename( const char *path ) {
    if ( path == nullptr || path[0] == '\0' ) {
        return "";
    }
    
    const char *basename = path;
    
    for ( const char *it = path; *it != '\0'; ++it ) {
        if ( *it == '/' || *it == '\\' ) {
            basename = it + 1;
        }
    }   
    
    return basename;
}

/*
================
CypherSystem_TimeNowSeconds
================
*/
common::com_f64 CypherSystem_TimeNowSeconds() {
    const auto now = std::chrono::steady_clock::now();
    const auto seconds = std::chrono::duration<common::com_f64>( now.time_since_epoch() );
    return seconds.count();
}

/*
================
CypherSystem_LocalTime
================
*/
bool CypherSystem_LocalTime( std::time_t time_value, std::tm &time_out ) {

#   if      CYPHER_PLATFORM_WINDOWS 
                return localtime_s( &time_out, &time_value ) == 0;
#   else 
                return localtime_r( &time_value, &time_out ) != nullptr;
#   endif 
                
}

/*
================
CypherSystem_Paths
================
*/
const paths_t &CypherSystem_Paths() {
    return g_sys_runtime_state.sys_paths;
}

/*
================
CypherSystem_GetPaths
================
*/
error_code_t CypherSystem_GetPaths( paths_t &out_paths ) {
    if ( !g_sys_runtime_state.initialized ) {
        return error_code_t::ERR_NOT_INIT;
    }   
    
    out_paths = g_sys_runtime_state.sys_paths;
    
    return error_code_t::OK;
}
 
/*
================
CypherSystem_SleepMilliseconds
================
*/
void CypherSystem_SleepMilliseconds( common::u64 milliseconds ) {
    CypherSystem_PlatformSleepMilliseconds( milliseconds );
    return ;
}

/*
================
CypherSystem_VirtualPageSize
================
*/
common::usize CypherSystem_VirtualPageSize()
{
    return CypherSystem_PlatformVirtualPageSize();
}

/*
================
CypherSystem_VirtualReserve
================
*/
void *CypherSystem_VirtualReserve( const common::usize size )
{
    return CypherSystem_PlatformVirtualReserve( CypherSystem_AlignVirtualMemorySize( size ) );
}

/*
================
CypherSystem_VirtualCommit
================
*/
error_code_t CypherSystem_VirtualCommit( void *memory, common::usize size )
{
    if ( CypherSystem_PlatformVirtualCommit( memory, CypherSystem_AlignVirtualMemorySize( size ) ) ) {
        return error_code_t::OK;
    }

    return error_code_t::ERR_INTERNAL_ERROR;
}

/*
================
CypherSystem_VirtualDecommit
================
*/
error_code_t CypherSystem_VirtualDecommit( void *memory, common::usize size )
{
    if ( CypherSystem_PlatformVirtualDecommit( memory, CypherSystem_AlignVirtualMemorySize( size ) ) ) {
        return error_code_t::OK;
    }

    return error_code_t::ERR_INTERNAL_ERROR;
}

/*
================
CypherSystem_VirtualRelease
================
*/
error_code_t CypherSystem_VirtualRelease( void *memory, common::usize size )
{
    if ( CypherSystem_PlatformVirtualRelease( memory, CypherSystem_AlignVirtualMemorySize( size ) ) ) {
        return error_code_t::OK;
    }

    return error_code_t::ERR_INTERNAL_ERROR;
}

}       // namespace cypher::engine::sys

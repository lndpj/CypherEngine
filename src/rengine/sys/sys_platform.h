#pragma once

#include "rengine/sys/sys_error.h"
#include "rengine/rcommon/com_main.h"

#include <ctime>       // std::time_t / std::tm for local time conversion.

namespace reap::rengine::sys
{

constexpr rcommon::u32 SYS_MAX_PATH_LENGTH = 1024u;
constexpr rcommon::u32 SYS_MAX_NAME_LENGTH = 256;

/*
================
Platform Detection Types
================
*/
enum class platform_t : rcommon::com_u8 {
    UNKNOWN = 0,
    WINDOWS,
    LINUX,
    MACOSX
};

enum class compiler_t : rcommon::com_u8 {
    UNKNOWN = 0,
    CLANG,
    GCC,
    MSVC
};

/*
================
System Startup Data
================
*/
struct sys_init_info_t {
    int argc{ 0 };
    const char *const *argv{ nullptr };

    const char *app_name{ nullptr };
    const char *organization_name{ nullptr };
};

struct sys_paths_t {
    char executable_path[SYS_MAX_PATH_LENGTH]{};
    char executable_dir[SYS_MAX_PATH_LENGTH]{};
    char working_dir[SYS_MAX_PATH_LENGTH]{};

    char base_path[SYS_MAX_PATH_LENGTH]{};
    char user_path[SYS_MAX_PATH_LENGTH]{};
};

struct sys_runtime_state_t {
    bool initialized{ false };
    
    char app_name[SYS_MAX_NAME_LENGTH]{};
    char organization_name[SYS_MAX_NAME_LENGTH]{};
    
    int argc{ 0u };
    const char *const *argv{ nullptr };
    sys_paths_t sys_paths{};
    
};

extern sys_runtime_state_t g_sys_runtime_state;

/*
================
System API
================
*/
sys_error_code_t Sys_Init( const sys_init_info_t &init_info );
sys_error_code_t Sys_Shutdown();

bool Sys_IsInitialized();

platform_t Sys_PlatformType();
compiler_t Sys_CompilerType();

const char *Sys_PlatformName( platform_t type );
const char *Sys_CompilerName( compiler_t type );

const sys_paths_t &Sys_Paths();
sys_error_code_t Sys_GetPaths( sys_paths_t &out_paths );

const char *Sys_PathBasename( const char *path );

rcommon::com_f64 Sys_TimeNowSeconds();
void Sys_SleepMilliseconds( rcommon::u64 milliseconds );

bool Sys_LocalTime( std::time_t time_value, std::tm &time_out );

/*
================
Cross Platform And Compiler Detection
================
*/

#   if  defined( _WIN32 ) || defined( __WIN32__ ) || defined( WIN32 ) || defined( MINGW32 )
#       define REAP_PLATFORM_WINDOWS    1
#   else
#       define REAP_PLATFORM_WINDOWS    0
#   endif

#   if  defined( __APPLE__ ) && defined( __MACH__ )
#       define REAP_PLATFORM_MACOS      1
#   else
#       define REAP_PLATFORM_MACOS      0
#   endif

#   if  defined( __linux__ ) 
#       define REAP_PLATFORM_LINUX      1
#   else
#       define REAP_PLATFORM_LINUX      0
#   endif

#   if  defined( _MSC_VER ) 
#       define REAP_COMPILER_MSVC       1
#   else
#       define REAP_COMPILER_MSVC       0
#   endif

#   if  defined( __clang__ ) 
#       define REAP_COMPILER_CLANG      1
#   else
#       define REAP_COMPILER_CLANG      0
#   endif

#   if  defined( __GNUC__ ) && !defined( __clang__ ) 
#       define REAP_COMPILER_GCC        1
#   else
#       define REAP_COMPILER_GCC        0
#   endif

}

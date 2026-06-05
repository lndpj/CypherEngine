#pragma once

#include "CypherEngine/CypherSystem/CypherSystem_Error.h"
#include "CypherEngine/CypherCommon/CypherCommon.h"

#include <ctime>       // std::time_t / std::tm for local time conversion.

namespace cypher::engine::sys
{

constexpr common::u32 SYS_MAX_PATH_LENGTH = 1024u;
constexpr common::u32 SYS_MAX_NAME_LENGTH = 256;

/*
================
Platform Detection Types
================
*/
enum class platform_t : common::com_u8 {
    UNKNOWN = 0,
    WINDOWS,
    LINUX,
    MACOSX
};

enum class compiler_t : common::com_u8 {
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
struct init_info_t {
    int argc{ 0 };
    const char *const *argv{ nullptr };

    const char *app_name{ nullptr };
    const char *organization_name{ nullptr };
};

struct paths_t {
    char executable_path[SYS_MAX_PATH_LENGTH]{};
    char executable_dir[SYS_MAX_PATH_LENGTH]{};
    char working_dir[SYS_MAX_PATH_LENGTH]{};

    char base_path[SYS_MAX_PATH_LENGTH]{};
    char user_path[SYS_MAX_PATH_LENGTH]{};
};

struct runtime_state_t {
    bool initialized{ false };
    
    char app_name[SYS_MAX_NAME_LENGTH]{};
    char organization_name[SYS_MAX_NAME_LENGTH]{};
    
    int argc{ 0u };
    const char *const *argv{ nullptr };
    paths_t sys_paths{};
    
};

extern runtime_state_t g_sys_runtime_state;

/*
================
System API
================
*/
error_code_t CypherSystem_Init( const init_info_t &init_info );
error_code_t CypherSystem_Shutdown();

bool CypherSystem_IsInitialized();

platform_t CypherSystem_PlatformType();
compiler_t CypherSystem_CompilerType();

const char *CypherSystem_PlatformName( platform_t type );
const char *CypherSystem_CompilerName( compiler_t type );

const paths_t &CypherSystem_Paths();
error_code_t CypherSystem_GetPaths( paths_t &out_paths );

const char *CypherSystem_PathBasename( const char *path );

common::com_f64 CypherSystem_TimeNowSeconds();
void CypherSystem_SleepMilliseconds( common::u64 milliseconds );

bool CypherSystem_LocalTime( std::time_t time_value, std::tm &time_out );

/*
================
Cross Platform And Compiler Detection
================
*/

#   if  defined( _WIN32 ) || defined( __WIN32__ ) || defined( WIN32 ) || defined( MINGW32 )
#       define CYPHER_PLATFORM_WINDOWS    1
#   else
#       define CYPHER_PLATFORM_WINDOWS    0
#   endif

#   if  defined( __APPLE__ ) && defined( __MACH__ )
#       define CYPHER_PLATFORM_MACOS      1
#   else
#       define CYPHER_PLATFORM_MACOS      0
#   endif

#   if  defined( __linux__ ) 
#       define CYPHER_PLATFORM_LINUX      1
#   else
#       define CYPHER_PLATFORM_LINUX      0
#   endif

#   if  defined( _MSC_VER ) 
#       define CYPHER_COMPILER_MSVC       1
#   else
#       define CYPHER_COMPILER_MSVC       0
#   endif

#   if  defined( __clang__ ) 
#       define CYPHER_COMPILER_CLANG      1
#   else
#       define CYPHER_COMPILER_CLANG      0
#   endif

#   if  defined( __GNUC__ ) && !defined( __clang__ ) 
#       define CYPHER_COMPILER_GCC        1
#   else
#       define CYPHER_COMPILER_GCC        0
#   endif

}       // namespace cypher::engine::sys

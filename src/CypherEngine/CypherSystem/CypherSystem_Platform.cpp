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
#include "CypherEngine/CypherLog/CypherLog.h"

#ifdef CYPHER_PLATFORM_WINDOWS
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif

    #ifndef NOMINMAX
        #define NOMINMAX
    #endif

    #include <windows.h>    // Win32 path, sleep and virtual memory APIs.
#endif

#ifdef CYPHER_PLATFORM_MACOS
    #include <mach-o/dyld.h> // _NSGetExecutablePath.
#endif

#if defined( CYPHER_PLATFORM_MACOS ) || defined( CYPHER_PLATFORM_LINUX )
    #include <sys/mman.h>   // mmap / mprotect / madvise / munmap.
    #include <unistd.h>     // readlink / nanosleep / sysconf.
#endif

#include <cerrno>       // errno / EINTR.
#include <chrono>      // steady_clock timing.
#include <cstdint>     // std::uint32_t for macOS executable API.
#include <cstdlib>     // getenv.
#include <cstring>     // strcmp / strncpy for path buffers.
#include <ctime>       // nanosleep / timespec.
#include <filesystem>  // Path normalization and directory creation.
#include <string>      // Temporary path strings.
#include <system_error> // std::error_code for non-throwing filesystem calls.

namespace cypher::engine::sys
{

runtime_state_t g_sys_runtime_state;

namespace {

/*
================
CypherSystem_CopyPath
================
*/
bool CypherSystem_CopyPath( char *out_path, const common::u32 out_path_size, const std::filesystem::path &path )
{
    if ( out_path == nullptr || out_path_size == 0u ) {
        return false;
    }

    std::string path_string = path.lexically_normal().string();

    if ( path_string.size() >= out_path_size ) {
        out_path[0] = '\0';
        return false;
    }

    std::strncpy( out_path, path_string.c_str(), out_path_size - 1u );
    out_path[out_path_size - 1u] = '\0';

    return true;
}

/*
================
CypherSystem_FindArgvValue
================
*/
const char *CypherSystem_FindArgvValue( const init_info_t &info, const char *argv_name )
{
    if ( info.argv == nullptr || argv_name == nullptr ) {
        return nullptr;
    }

    for ( int i = 1; i + 1 < info.argc; ++i ) {
        if ( std::strcmp( info.argv[i], argv_name ) == 0 ) {
            return info.argv[i + 1];
        }
    }

    return nullptr;
}

sys_error_t CypherSystem_PlatformBuildPaths( const init_info_t &info_init, paths_t &out_paths );
void CypherSystem_PlatformSleepMilliseconds( common::u64 milliseconds );
common::usize CypherSystem_PlatformVirtualPageSize();
void *CypherSystem_PlatformVirtualReserve( common::usize size );
bool CypherSystem_PlatformVirtualCommit( void *memory, common::usize size );
bool CypherSystem_PlatformVirtualDecommit( void *memory, common::usize size );
bool CypherSystem_PlatformVirtualRelease( void *memory, common::usize size );

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
#   ifdef CYPHER_PLATFORM_WINDOWS
                return platform_t::WINDOWS;
#   elif defined( CYPHER_PLATFORM_MACOS )
                return platform_t::MACOSX;
#   elif defined( CYPHER_PLATFORM_LINUX )
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
#   ifdef CYPHER_COMPILER_MSVC
                return compiler_t::MSVC;
#   elif defined( CYPHER_COMPILER_CLANG )
                return compiler_t::CLANG;
#   elif defined( CYPHER_COMPILER_GCC )
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
sys_error_t CypherSystem_Init( const init_info_t &info_init ) {
    if ( g_sys_runtime_state.initialized ) {
        return sys_error_t::ERR_IS_INIT;
    }
    if ( info_init.app_name == nullptr || info_init.app_name[0] == '\0' ) {
        return sys_error_t::ERR_INVALID_ARGUMENT;
    }
    if ( info_init.organization_name == nullptr || info_init.organization_name[0] == '\0' ) {
        return sys_error_t::ERR_INVALID_ARGUMENT;
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
    
    sys_error_t paths_result = CypherSystem_PlatformBuildPaths( info_init, g_sys_runtime_state.sys_paths );
    
    if ( paths_result != sys_error_t::OK ) {
        g_sys_runtime_state = {};
        return paths_result;
    }
    
    g_sys_runtime_state.initialized = true;
    
    return sys_error_t::OK;
}

/*
================
CypherSystem_Shutdown
================
*/
sys_error_t CypherSystem_Shutdown() {
    if ( !g_sys_runtime_state.initialized ) {
        return sys_error_t::ERR_NOT_INIT;
    }
    
    g_sys_runtime_state = {};
    
    g_sys_runtime_state.initialized = false;
    
    return sys_error_t::OK;
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
        case platform_t::WINDOWS:   return "Windows";
    case platform_t::LINUX:         return "Linux";
    case platform_t::MACOSX:        return "MacOS";
        default:                    return "Unknown";
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
        case compiler_t::GCC:   return "GCC";
        case compiler_t::MSVC:  return "MSVC";
        default:                return "Unknown";
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

#   ifdef CYPHER_PLATFORM_WINDOWS
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
sys_error_t CypherSystem_GetPaths( paths_t &out_paths ) {
    if ( !g_sys_runtime_state.initialized ) {
        return sys_error_t::ERR_NOT_INIT;
    }   
    
    out_paths = g_sys_runtime_state.sys_paths;
    
    return sys_error_t::OK;
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
sys_error_t CypherSystem_VirtualCommit( void *memory, common::usize size )
{
    if ( CypherSystem_PlatformVirtualCommit( memory, CypherSystem_AlignVirtualMemorySize( size ) ) ) {
        return sys_error_t::OK;
    }

    return sys_error_t::ERR_INTERNAL_ERROR;
}

/*
================
CypherSystem_VirtualDecommit
================
*/
sys_error_t CypherSystem_VirtualDecommit( void *memory, common::usize size )
{
    if ( CypherSystem_PlatformVirtualDecommit( memory, CypherSystem_AlignVirtualMemorySize( size ) ) ) {
        return sys_error_t::OK;
    }

    return sys_error_t::ERR_INTERNAL_ERROR;
}

/*
================
CypherSystem_VirtualRelease
================
*/
sys_error_t CypherSystem_VirtualRelease( void *memory, common::usize size )
{
    if ( CypherSystem_PlatformVirtualRelease( memory, CypherSystem_AlignVirtualMemorySize( size ) ) ) {
        return sys_error_t::OK;
    }

    return sys_error_t::ERR_INTERNAL_ERROR;
}

namespace {

#ifdef CYPHER_PLATFORM_WINDOWS

/*
================
CypherSystem_PlatformBuildPaths

Builds Win32 executable, base and user paths.
================
*/
sys_error_t CypherSystem_PlatformBuildPaths( const init_info_t &info_init, paths_t &out_paths )
{
    out_paths = {};

    std::error_code ec{};

    const std::filesystem::path working_dir = std::filesystem::current_path( ec );
    if ( ec ) {
        return sys_error_t::ERR_PATH_QUERY_FAILED;
    }

    char executable_buffer[SYS_MAX_PATH_LENGTH]{};

    const DWORD executable_length = GetModuleFileNameA(
        nullptr,
        executable_buffer,
        static_cast<DWORD>( sizeof( executable_buffer ) )
    );

    if ( executable_length == 0u ) {
        return sys_error_t::ERR_PATH_QUERY_FAILED;
    }

    if ( executable_length >= sizeof( executable_buffer ) ) {
        return sys_error_t::ERR_PATH_TOO_LONG;
    }

    executable_buffer[executable_length] = '\0';

    std::filesystem::path executable_path = std::filesystem::weakly_canonical( executable_buffer, ec );

    if ( ec ) {
        ec.clear();
        executable_path = executable_buffer;
    }

    const std::filesystem::path executable_dir = executable_path.parent_path();
    const char *base_path_override = CypherSystem_FindArgvValue( info_init, "-basedir" );

    const std::filesystem::path base_path =
        ( base_path_override != nullptr && base_path_override[0] != '\0' ) ? std::filesystem::path( base_path_override ) : working_dir;

    const char *user_path_override = CypherSystem_FindArgvValue( info_init, "-userpath" );
    std::filesystem::path user_path{};

    if ( user_path_override != nullptr && user_path_override[0] != '\0' ) {
        user_path = user_path_override;
    } else {
        char appdata_buffer[SYS_MAX_PATH_LENGTH]{};

        const DWORD appdata_length = GetEnvironmentVariableA(
            "APPDATA",
            appdata_buffer,
            static_cast<DWORD>( sizeof( appdata_buffer ) )
        );

        if ( appdata_length == 0u ) {
            return sys_error_t::ERR_PATH_QUERY_FAILED;
        }

        if ( appdata_length >= sizeof( appdata_buffer ) ) {
            return sys_error_t::ERR_PATH_TOO_LONG;
        }

        appdata_buffer[appdata_length] = '\0';

        user_path = std::filesystem::path( appdata_buffer ) / info_init.app_name;
    }

    std::filesystem::create_directories( user_path, ec );
    if ( ec ) {
        return sys_error_t::ERR_DIRECTORY_CREATE_FAILED;
    }

    if ( !CypherSystem_CopyPath( out_paths.executable_path, sizeof( out_paths.executable_path ), executable_path ) ) {
        return sys_error_t::ERR_PATH_TOO_LONG;
    }

    if ( !CypherSystem_CopyPath( out_paths.executable_dir, sizeof( out_paths.executable_dir ), executable_dir ) ) {
        return sys_error_t::ERR_PATH_TOO_LONG;
    }

    if ( !CypherSystem_CopyPath( out_paths.working_dir, sizeof( out_paths.working_dir ), working_dir ) ) {
        return sys_error_t::ERR_PATH_TOO_LONG;
    }

    if ( !CypherSystem_CopyPath( out_paths.base_path, sizeof( out_paths.base_path ), base_path ) ) {
        return sys_error_t::ERR_PATH_TOO_LONG;
    }

    if ( !CypherSystem_CopyPath( out_paths.user_path, sizeof( out_paths.user_path ), user_path ) ) {
        return sys_error_t::ERR_PATH_TOO_LONG;
    }

    return sys_error_t::OK;
}

/*
================
CypherSystem_PlatformSleepMilliseconds
================
*/
void CypherSystem_PlatformSleepMilliseconds( common::u64 milliseconds )
{
    Sleep( static_cast<DWORD>( milliseconds ) );
}

/*
================
CypherSystem_PlatformVirtualPageSize
================
*/
common::usize CypherSystem_PlatformVirtualPageSize()
{
    constexpr common::usize DEFAULT_PAGE_SIZE = 4096u;
    SYSTEM_INFO info{};
    GetSystemInfo( &info );

    if ( info.dwPageSize == 0u ) {
        LOG_WARNING( log::channel_t::PLATFORM, "GetSystemInfo page size query failed; using default page size %zu.", DEFAULT_PAGE_SIZE );
        return DEFAULT_PAGE_SIZE;
    }

    return static_cast<common::usize>( info.dwPageSize );
}

/*
================
CypherSystem_PlatformVirtualReserve
================
*/
void *CypherSystem_PlatformVirtualReserve( common::usize size )
{
    if ( size == 0u ) {
        LOG_ERROR( log::channel_t::PLATFORM, "virtual reserve failed: requested size is zero." );
        return nullptr;
    }

    void *memory = VirtualAlloc( nullptr, size, MEM_RESERVE, PAGE_NOACCESS );

    if ( memory == nullptr ) {
        const DWORD error = GetLastError();
        LOG_ERROR( log::channel_t::PLATFORM, "virtual reserve failed: size=%zu, win32_error=%lu.", size, static_cast<unsigned long>( error ) );
        return nullptr;
    }

    return memory;
}

/*
================
CypherSystem_PlatformVirtualCommit
================
*/
bool CypherSystem_PlatformVirtualCommit( void *memory, common::usize size )
{
    if ( memory == nullptr || size == 0u ) {
        LOG_ERROR( log::channel_t::PLATFORM, "virtual commit failed: memory=%p, size=%zu.", memory, size );
        return false;
    }

    void *result = VirtualAlloc( memory, size, MEM_COMMIT, PAGE_READWRITE );
    if ( result == nullptr ) {
        const DWORD error = GetLastError();
        LOG_ERROR( log::channel_t::PLATFORM, "virtual commit failed: memory=%p, size=%zu, win32_error=%lu.", memory, size, static_cast<unsigned long>( error ) );
        return false;
    }

    return true;
}

/*
================
CypherSystem_PlatformVirtualDecommit
================
*/
bool CypherSystem_PlatformVirtualDecommit( void *memory, common::usize size )
{
    if ( memory == nullptr || size == 0u ) {
        LOG_ERROR( log::channel_t::PLATFORM, "virtual decommit failed: memory=%p, size=%zu.", memory, size );
        return false;
    }

    BOOL result = VirtualFree( memory, size, MEM_DECOMMIT );
    if ( result == 0 ) {
        const DWORD error = GetLastError();
        LOG_ERROR( log::channel_t::PLATFORM, "virtual decommit failed: memory=%p, size=%zu, win32_error=%lu.", memory, size, static_cast<unsigned long>( error ) );
        return false;
    }

    return true;
}

/*
================
CypherSystem_PlatformVirtualRelease
================
*/
bool CypherSystem_PlatformVirtualRelease( void *memory, common::usize size )
{
    if ( memory == nullptr || size == 0u ) {
        LOG_ERROR( log::channel_t::PLATFORM, "virtual release failed: memory=%p, size=%zu.", memory, size );
        return false;
    }

    BOOL result = VirtualFree( memory, 0, MEM_RELEASE );
    if ( result == 0 ) {
        const DWORD error = GetLastError();
        LOG_ERROR( log::channel_t::PLATFORM, "virtual release failed: memory=%p, size=%zu, win32_error=%lu.", memory, size, static_cast<unsigned long>( error ) );
        return false;
    }

    return true;
}

#endif          // CYPHER_PLATFORM_WINDOWS

#ifdef CYPHER_PLATFORM_MACOS

/*
================
CypherSystem_PlatformBuildPaths

Builds macOS executable, base and user paths.
================
*/
sys_error_t CypherSystem_PlatformBuildPaths( const init_info_t &info_init, paths_t &out_paths )
{
    out_paths = {};

    std::error_code ec{};

    const std::filesystem::path working_dir = std::filesystem::current_path( ec );
    if ( ec ) {
        return sys_error_t::ERR_PATH_QUERY_FAILED;
    }

    char executable_buffer[SYS_MAX_PATH_LENGTH]{};
    std::uint32_t executable_buffer_size = static_cast<std::uint32_t>( sizeof( executable_buffer ) );

    if ( _NSGetExecutablePath( executable_buffer, &executable_buffer_size ) != 0 ) {
        return sys_error_t::ERR_PATH_TOO_LONG;
    }

    std::filesystem::path executable_path = std::filesystem::weakly_canonical( executable_buffer, ec );

    if ( ec ) {
        ec.clear();
        executable_path = executable_buffer;
    }

    const std::filesystem::path executable_dir = executable_path.parent_path();
    const char *base_path_override = CypherSystem_FindArgvValue( info_init, "-basedir" );

    const std::filesystem::path base_path =
        ( base_path_override != nullptr && base_path_override[0] != '\0' ) ? std::filesystem::path( base_path_override ) : working_dir;

    const char *user_path_override = CypherSystem_FindArgvValue( info_init, "-userpath" );

    std::filesystem::path user_path{};

    if ( user_path_override != nullptr && user_path_override[0] != '\0' ) {
        user_path = user_path_override;
    } else {
        const char *home = std::getenv( "HOME" );

        if ( home == nullptr || home[0] == '\0' ) {
            return sys_error_t::ERR_PATH_QUERY_FAILED;
        }

        user_path = std::filesystem::path( home ) /
                    "Library" /
                    "Application Support" /
                    info_init.app_name;
    }

    std::filesystem::create_directories( user_path, ec );
    if ( ec ) {
        return sys_error_t::ERR_DIRECTORY_CREATE_FAILED;
    }

    if ( !CypherSystem_CopyPath( out_paths.executable_path, sizeof( out_paths.executable_path ), executable_path ) ) {
        return sys_error_t::ERR_PATH_TOO_LONG;
    }

    if ( !CypherSystem_CopyPath( out_paths.executable_dir, sizeof( out_paths.executable_dir ), executable_dir ) ) {
        return sys_error_t::ERR_PATH_TOO_LONG;
    }

    if ( !CypherSystem_CopyPath( out_paths.working_dir, sizeof( out_paths.working_dir ), working_dir ) ) {
        return sys_error_t::ERR_PATH_TOO_LONG;
    }

    if ( !CypherSystem_CopyPath( out_paths.base_path, sizeof( out_paths.base_path ), base_path ) ) {
        return sys_error_t::ERR_PATH_TOO_LONG;
    }

    if ( !CypherSystem_CopyPath( out_paths.user_path, sizeof( out_paths.user_path ), user_path ) ) {
        return sys_error_t::ERR_PATH_TOO_LONG;
    }

    return sys_error_t::OK;
}

/*
================
CypherSystem_PlatformSleepMilliseconds
================
*/
void CypherSystem_PlatformSleepMilliseconds( common::u64 milliseconds )
{
    timespec request{};
    request.tv_sec = static_cast<time_t>( milliseconds / 1000u );
    request.tv_nsec = static_cast<long>( ( milliseconds % 1000u ) * 1000000u );

    while ( nanosleep( &request, &request ) == -1 && errno == EINTR ) {
    }
}

/*
================
CypherSystem_PlatformVirtualPageSize
================
*/
common::usize CypherSystem_PlatformVirtualPageSize()
{
    constexpr common::usize DEFAULT_PAGE_SIZE = 4096u;

    const long page_size = sysconf( _SC_PAGESIZE );
    if ( page_size <= 0 ) {
        LOG_WARNING( log::channel_t::PLATFORM, "sysconf(_SC_PAGESIZE) failed; using default page size %zu.", DEFAULT_PAGE_SIZE );
        return DEFAULT_PAGE_SIZE;
    }

    return static_cast<common::usize>( page_size );
}

/*
================
CypherSystem_PlatformVirtualReserve
================
*/
void *CypherSystem_PlatformVirtualReserve( common::usize size )
{
    if ( size == 0u ) {
        LOG_ERROR( log::channel_t::PLATFORM, "virtual reserve failed: requested size is zero." );
        return nullptr;
    }

    void *memory = mmap(
        nullptr,
        size,
        PROT_NONE,
        MAP_PRIVATE | MAP_ANON,
        -1,
        0
    );

    if ( memory == MAP_FAILED ) {
        LOG_ERROR( log::channel_t::PLATFORM, "virtual reserve failed: size=%zu, errno=%d.", size, errno );
        return nullptr;
    }

    return memory;
}

/*
================
CypherSystem_PlatformVirtualCommit
================
*/
bool CypherSystem_PlatformVirtualCommit( void *memory, common::usize size )
{
    if ( memory == nullptr || size == 0u ) {
        LOG_ERROR( log::channel_t::PLATFORM, "virtual commit failed: memory=%p, size=%zu.", memory, size );
        return false;
    }

    const int result = mprotect( memory, size, PROT_READ | PROT_WRITE );
    if ( result != 0 ) {
        LOG_ERROR( log::channel_t::PLATFORM, "virtual commit failed: memory=%p, size=%zu, errno=%d.", memory, size, errno );
        return false;
    }

    return true;
}

/*
================
CypherSystem_PlatformVirtualDecommit
================
*/
bool CypherSystem_PlatformVirtualDecommit( void *memory, common::usize size )
{
    if ( memory == nullptr || size == 0u ) {
        LOG_ERROR( log::channel_t::PLATFORM, "virtual decommit failed: memory=%p, size=%zu.", memory, size );
        return false;
    }

    int result = madvise( memory, size, MADV_FREE );
    if ( result != 0 ) {
        LOG_WARNING( log::channel_t::PLATFORM, "virtual decommit madvise warning: memory=%p, size=%zu, errno=%d.", memory, size, errno );
    }

    result = mprotect( memory, size, PROT_NONE );
    if ( result != 0 ) {
        LOG_ERROR( log::channel_t::PLATFORM, "virtual decommit failed: memory=%p, size=%zu, errno=%d.", memory, size, errno );
        return false;
    }

    return true;
}

/*
================
CypherSystem_PlatformVirtualRelease
================
*/
bool CypherSystem_PlatformVirtualRelease( void *memory, common::usize size )
{
    if ( memory == nullptr || size == 0u ) {
        LOG_ERROR( log::channel_t::PLATFORM, "virtual release failed: memory=%p, size=%zu.", memory, size );
        return false;
    }

    const int result = munmap( memory, size );
    if ( result != 0 ) {
        LOG_ERROR( log::channel_t::PLATFORM, "virtual release failed: memory=%p, size=%zu, errno=%d.", memory, size, errno );
        return false;
    }

    return true;
}

#endif          // CYPHER_PLATFORM_MACOS

#ifdef CYPHER_PLATFORM_LINUX

/*
================
CypherSystem_PlatformBuildPaths

Builds Linux executable, base and user paths.
================
*/
sys_error_t CypherSystem_PlatformBuildPaths( const init_info_t &info_init, paths_t &out_paths )
{
    std::error_code ec{};
    out_paths = {};

    const std::filesystem::path working_dir = std::filesystem::current_path( ec );
    if ( ec ) {
        return sys_error_t::ERR_PATH_QUERY_FAILED;
    }

    char executable_buffer[SYS_MAX_PATH_LENGTH]{};

    const ssize_t executable_length = readlink(
        "/proc/self/exe",
        executable_buffer,
        sizeof( executable_buffer ) - 1u
    );

    if ( executable_length < 0 ) {
        return sys_error_t::ERR_PATH_QUERY_FAILED;
    }

    executable_buffer[executable_length] = '\0';

    std::filesystem::path executable_path = std::filesystem::weakly_canonical( executable_buffer, ec );

    if ( ec ) {
        ec.clear();
        executable_path = executable_buffer;
    }

    const std::filesystem::path executable_dir = executable_path.parent_path();
    const char *base_path_override = CypherSystem_FindArgvValue( info_init, "-basedir" );

    const std::filesystem::path base_path =
        ( base_path_override != nullptr && base_path_override[0] != '\0' ) ? std::filesystem::path( base_path_override ) : working_dir;

    const char *user_path_override = CypherSystem_FindArgvValue( info_init, "-userpath" );

    std::filesystem::path user_path{};

    if ( user_path_override != nullptr && user_path_override[0] != '\0' ) {
        user_path = user_path_override;
    } else {
        const char *home = std::getenv( "HOME" );

        if ( home == nullptr || home[0] == '\0' ) {
            return sys_error_t::ERR_PATH_QUERY_FAILED;
        }

        user_path = std::filesystem::path( home ) /
                    ".local" /
                    "share" /
                    info_init.app_name;
    }

    std::filesystem::create_directories( user_path, ec );

    if ( ec ) {
        return sys_error_t::ERR_DIRECTORY_CREATE_FAILED;
    }

    if ( !CypherSystem_CopyPath( out_paths.executable_path, sizeof( out_paths.executable_path ), executable_path ) ) {
        return sys_error_t::ERR_PATH_TOO_LONG;
    }

    if ( !CypherSystem_CopyPath( out_paths.executable_dir, sizeof( out_paths.executable_dir ), executable_dir ) ) {
        return sys_error_t::ERR_PATH_TOO_LONG;
    }

    if ( !CypherSystem_CopyPath( out_paths.working_dir, sizeof( out_paths.working_dir ), working_dir ) ) {
        return sys_error_t::ERR_PATH_TOO_LONG;
    }

    if ( !CypherSystem_CopyPath( out_paths.base_path, sizeof( out_paths.base_path ), base_path ) ) {
        return sys_error_t::ERR_PATH_TOO_LONG;
    }

    if ( !CypherSystem_CopyPath( out_paths.user_path, sizeof( out_paths.user_path ), user_path ) ) {
        return sys_error_t::ERR_PATH_TOO_LONG;
    }

    return sys_error_t::OK;
}

/*
================
CypherSystem_PlatformSleepMilliseconds
================
*/
void CypherSystem_PlatformSleepMilliseconds( common::u64 milliseconds )
{
    timespec request{};
    request.tv_sec = static_cast<time_t>( milliseconds / 1000u );
    request.tv_nsec = static_cast<long>( ( milliseconds % 1000u ) * 1000000u );

    while ( nanosleep( &request, &request ) == -1 && errno == EINTR ) {
    }
}

/*
================
CypherSystem_PlatformVirtualPageSize
================
*/
common::usize CypherSystem_PlatformVirtualPageSize()
{
    constexpr common::usize DEFAULT_PAGE_SIZE = 4096u;
    const long page_size = sysconf( _SC_PAGESIZE );

    if ( page_size <= 0 ) {
        LOG_WARNING( log::channel_t::PLATFORM, "sysconf(_SC_PAGESIZE) failed; using default page size %zu.", DEFAULT_PAGE_SIZE );
        return DEFAULT_PAGE_SIZE;
    }

    return static_cast<common::usize>( page_size );
}

/*
================
CypherSystem_PlatformVirtualReserve
================
*/
void *CypherSystem_PlatformVirtualReserve( common::usize size )
{
    if ( size == 0u ) {
        LOG_ERROR( log::channel_t::PLATFORM, "virtual reserve failed: requested size is zero." );
        return nullptr;
    }

    void *memory = mmap(
        nullptr,
        size,
        PROT_NONE,
        MAP_PRIVATE | MAP_ANONYMOUS,
        -1,
        0
    );

    if ( memory == MAP_FAILED ) {
        LOG_ERROR( log::channel_t::PLATFORM, "virtual reserve failed: size=%zu, errno=%d.", size, errno );
        return nullptr;
    }

    return memory;
}

/*
================
CypherSystem_PlatformVirtualCommit
================
*/
bool CypherSystem_PlatformVirtualCommit( void *memory, common::usize size )
{
    if ( memory == nullptr || size == 0u ) {
        LOG_ERROR( log::channel_t::PLATFORM, "virtual commit failed: memory=%p, size=%zu.", memory, size );
        return false;
    }

    const int result = mprotect( memory, size, PROT_READ | PROT_WRITE );
    if ( result != 0 ) {
        LOG_ERROR( log::channel_t::PLATFORM, "virtual commit failed: memory=%p, size=%zu, errno=%d.", memory, size, errno );
        return false;
    }

    return true;
}

/*
================
CypherSystem_PlatformVirtualDecommit
================
*/
bool CypherSystem_PlatformVirtualDecommit( void *memory, common::usize size )
{
    if ( memory == nullptr || size == 0u ) {
        LOG_ERROR( log::channel_t::PLATFORM, "virtual decommit failed: memory=%p, size=%zu.", memory, size );
        return false;
    }

    int result = madvise( memory, size, MADV_DONTNEED );
    if ( result != 0 ) {
        LOG_WARNING( log::channel_t::PLATFORM, "virtual decommit madvise warning: memory=%p, size=%zu, errno=%d.", memory, size, errno );
    }

    result = mprotect( memory, size, PROT_NONE );
    if ( result != 0 ) {
        LOG_ERROR( log::channel_t::PLATFORM, "virtual decommit failed: memory=%p, size=%zu, errno=%d.", memory, size, errno );
        return false;
    }

    return true;
}

/*
================
CypherSystem_PlatformVirtualRelease
================
*/
bool CypherSystem_PlatformVirtualRelease( void *memory, common::usize size )
{
    if ( memory == nullptr || size == 0u ) {
        LOG_ERROR( log::channel_t::PLATFORM, "virtual release failed: memory=%p, size=%zu.", memory, size );
        return false;
    }

    const int result = munmap( memory, size );
    if ( result != 0 ) {
        LOG_ERROR( log::channel_t::PLATFORM, "virtual release failed: memory=%p, size=%zu, errno=%d.", memory, size, errno );
        return false;
    }

    return true;
}

#endif          // CYPHER_PLATFORM_LINUX

}               // namespace

}               // namespace cypher::engine::sys

/*======================================================================
   File: CypherSystem_PlatformLinux.cpp
   Project: CypherEngine
   Author: ksiric <email@example.com>
   Created: 2026-04-27 17:32:41
   Last Modified by: ksiric
   Last Modified: 2026-06-10 10:33:44
   ---------------------------------------------------------------------
   Description:

   ---------------------------------------------------------------------
   License:
   Company:
   Version: 0.1.0
 ======================================================================
                                                                       */
#include "CypherEngine/CypherLog/CypherLog.h"
#include "CypherEngine/CypherSystem/CypherSystem_PlatformInternal.h"

#if CYPHER_PLATFORM_LINUX

#include <chrono>          // Reserved for future platform timing extensions.
#include <cstdlib>         // getenv.
#include <cerrno>          // EINTR while sleeping.
#include <ctime>           // nanosleep / timespec.
#include <cstring>         // strcmp / strncpy for path buffers.
#include <filesystem>      // Path normalization and directory creation.
#include <string>          // Temporary path strings.
#include <system_error>    // std::error_code for non-throwing filesystem calls.
#include <sys/mman.h>      // mmap / mprotect / madvise / munmap.
#include <unistd.h>        // readlink / nanosleep platform headers.

namespace cypher::engine::sys
{

namespace {

/*
================
CypherSystem_CopyPath
================
*/
bool CypherSystem_CopyPath( char *out_path, const common::u32 out_path_size, const std::filesystem::path &path ) {
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
const char *CypherSystem_FindArgvValue( const init_info_t &info, const char *argv_name ) {
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

}

/*
================
CypherSystem_PlatformBuildPaths

Builds Linux executable, base and user paths.
================
*/
sys_error_t CypherSystem_PlatformBuildPaths(const init_info_t &info_init, paths_t &out_paths ) {
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
                    "share"  /
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
void CypherSystem_PlatformSleepMilliseconds( const common::u64 milliseconds ) {
    timespec request{};
    request.tv_sec = static_cast<time_t>( milliseconds / 1000u );
    request.tv_nsec = static_cast<long>( ( milliseconds % 1000u ) * 1000000u );

    while ( nanosleep( &request, &request ) == -1 && errno == EINTR ) {
    }
}

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


}       // namespace cypher::engine::sys

#endif

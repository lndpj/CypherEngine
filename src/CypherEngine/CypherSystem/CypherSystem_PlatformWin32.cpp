/*======================================================================
   File: sys_platform_win32.cpp
   Project: CypherEngine
   Author: ksiric <email@example.com>
   Created: 2026-04-27 17:32:49
   Last Modified by: ksiric
   Last Modified: 2026-05-04 23:40:45
   ---------------------------------------------------------------------
   Description:

   ---------------------------------------------------------------------
   License:
   Company:
   Version: 0.1.0
 ======================================================================
																	   */
#include "CypherEngine/CypherSystem/CypherSystem_PlatformInternal.h"

#if CYPHER_PLATFORM_WINDOWS

#ifndef WIN32_LEAN_AND_MEAN
#   define WIN32_LEAN_AND_MEAN
#endif

#ifndef NOMINMAX
#   define NOMINMAX
#endif

#include <windows.h>       // Win32 path and sleep APIs.

#include <cstdlib>         // Environment access compatibility.
#include <cstring>         // strcmp / strncpy for path buffers.
#include <filesystem>      // Path normalization and directory creation.
#include <string>          // Temporary path strings.
#include <system_error>    // std::error_code for non-throwing filesystem calls.

namespace cypher::engine::sys {

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

Builds Win32 executable, base and user paths.
================
*/
error_code_t CypherSystem_PlatformBuildPaths( const init_info_t &info_init, paths_t &out_paths ) {
	out_paths = {};

	std::error_code ec{};

	const std::filesystem::path working_dir = std::filesystem::current_path( ec );
	if ( ec ) {
		return error_code_t::ERR_PATH_QUERY_FAILED;
	}

    char executable_buffer[SYS_MAX_PATH_LENGTH]{};

    const DWORD executable_length = GetModuleFileNameA(
        nullptr,
        executable_buffer,
        static_cast<DWORD>( sizeof( executable_buffer ) )
    );

    if ( executable_length == 0u ) {
        return error_code_t::ERR_PATH_QUERY_FAILED;
    }

    if ( executable_length >= sizeof( executable_buffer ) ) {
        return error_code_t::ERR_PATH_TOO_LONG;
    }

    executable_buffer[executable_length] = '\0';

    std::filesystem::path executable_path =
        std::filesystem::weakly_canonical( executable_buffer, ec );

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
            return error_code_t::ERR_PATH_QUERY_FAILED;
        }

        if ( appdata_length >= sizeof( appdata_buffer ) ) {
            return error_code_t::ERR_PATH_TOO_LONG;
        }
        
        appdata_buffer[appdata_length] = '\0';

        user_path = std::filesystem::path( appdata_buffer ) / info_init.app_name;
    }

    std::filesystem::create_directories( user_path, ec ); 
    if ( ec ) {
        return error_code_t::ERR_DIRECTORY_CREATE_FAILED;
    } 

    if ( !CypherSystem_CopyPath( out_paths.executable_path, sizeof( out_paths.executable_path ), executable_path ) ) {
        return error_code_t::ERR_PATH_TOO_LONG;
    }

    if ( !CypherSystem_CopyPath( out_paths.executable_dir, sizeof( out_paths.executable_dir ), executable_dir ) ) {
        return error_code_t::ERR_PATH_TOO_LONG;
    }

    if ( !CypherSystem_CopyPath( out_paths.working_dir, sizeof( out_paths.working_dir ), working_dir ) ) {
        return error_code_t::ERR_PATH_TOO_LONG;
    }

    if ( !CypherSystem_CopyPath( out_paths.base_path, sizeof( out_paths.base_path ), base_path ) ) {
        return error_code_t::ERR_PATH_TOO_LONG;
    }

    if ( !CypherSystem_CopyPath( out_paths.user_path, sizeof( out_paths.user_path ), user_path ) ) {
        return error_code_t::ERR_PATH_TOO_LONG;
    }

    return error_code_t::OK;
}

/*
================
CypherSystem_PlatformSleepMilliseconds
================
*/
void CypherSystem_PlatformSleepMilliseconds( const common::u64 milliseconds ) {
	Sleep( static_cast<DWORD>( milliseconds ) );
}

} // namespace cypher::engine::sys

#endif

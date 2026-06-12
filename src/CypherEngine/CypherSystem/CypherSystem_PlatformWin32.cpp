/*======================================================================
   File: CypherSystem_PlatformWin32.cpp
   Project: CypherEngine
   Author: ksiric <email@example.com>
   Created: 2026-04-27 17:32:49
   Last Modified by: ksiric
   Last Modified: 2026-06-10 09:05:28
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

#if CYPHER_PLATFORM_WINDOWS

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <windows.h> // Win32 path and sleep APIs.

#include <cstdlib> // Environment access compatibility.
#include <cstring> // strcmp / strncpy for path buffers.
#include <filesystem> // Path normalization and directory creation.
#include <string> // Temporary path strings.
#include <system_error> // std::error_code for non-throwing filesystem calls.

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

} // namespace

/*
================
CypherSystem_PlatformBuildPaths

Builds Win32 executable, base and user paths.
================
*/
sys_error_t CypherSystem_PlatformBuildPaths( const init_info_t &info_init, paths_t &out_paths ) {
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
		static_cast<DWORD>( sizeof( executable_buffer ) ) );

	if ( executable_length == 0u ) {
		return sys_error_t::ERR_PATH_QUERY_FAILED;
	}

	if ( executable_length >= sizeof( executable_buffer ) ) {
		return sys_error_t::ERR_PATH_TOO_LONG;
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
			static_cast<DWORD>( sizeof( appdata_buffer ) ) );
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
void CypherSystem_PlatformSleepMilliseconds( const common::u64 milliseconds ) {
	Sleep( static_cast<DWORD>( milliseconds ) );
}

common::usize CypherSystem_PlatformVirtualPageSize() {
	constexpr common::usize DEFAULT_PAGE_SIZE = 4096u;
	SYSTEM_INFO info{};
	GetSystemInfo( &info );

	if ( info.dwPageSize == 0 ) {
		LOG_WARNING( log::channel_t::PLATFORM, "GetSystemInfo page size query failed; using default page size %zu.", DEFAULT_PAGE_SIZE );
		return DEFAULT_PAGE_SIZE;
	}

	return info.dwPageSize;
}

void *CypherSystem_PlatformVirtualReserve( common::usize size ) {
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

bool CypherSystem_PlatformVirtualCommit( void *memory, common::usize size ) {
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

bool CypherSystem_PlatformVirtualDecommit( void *memory, common::usize size ) {
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

bool CypherSystem_PlatformVirtualRelease( void *memory, common::usize size ) {
	if ( memory == nullptr || size == 0u ) {
		LOG_ERROR( log::channel_t::PLATFORM, "virtual release failed: memory=%p, size=%zu.", memory, size );
		return false;
	}
	/*
     * the size must be 0 when releasing the memory!
     */
	BOOL result = VirtualFree( memory, 0, MEM_RELEASE );
	if ( result == 0 ) {
		const DWORD error = GetLastError();
		LOG_ERROR( log::channel_t::PLATFORM, "virtual release failed: memory=%p, size=%zu, win32_error=%lu.", memory, size, static_cast<unsigned long>( error ) );
		return false;
	}
	return true;
}

} // namespace cypher::engine::sys

#endif

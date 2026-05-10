/*======================================================================
   File: fs_main.cpp
   Project: REAP
   Author: ksiric <email@example.com>
   Created: 2026-04-26 15:53:16
   Last Modified by: ksiric
   Last Modified: 2026-04-27 17:37:29
   ---------------------------------------------------------------------
   Description:

   ---------------------------------------------------------------------
   License:
   Company:
   Version: 0.1.0
 ======================================================================
																	   */

#include "rengine/fs/fs_main.h"

#include <cstdio>          // stdio file handles and read/write operations.
#include <cstring>         // strcmp / strncpy for fixed path buffers.
#include <filesystem>      // Path probing and directory creation.
#include <system_error>    // std::error_code for non-throwing filesystem calls.

namespace reap::rengine::fs {

namespace {

/*
================
Filesystem Runtime State
================
*/
struct fs_runtime_state_t {
	bool initialized{ false };
	fs_mount_t mounts[FS_MAX_MOUNTS]{};
	rcommon::u32 mount_count{ 0u };
	char write_path[FS_MAX_PATH_LENGTH]{};
};

fs_runtime_state_t g_fs_runtime_state{};

} // namespace

/*
================
FS_Init
================
*/
fs_error_code_t FS_Init() {
	if ( g_fs_runtime_state.initialized ) {
		return fs_error_code_t::ERR_IS_INIT;
	}

	g_fs_runtime_state = {};
	g_fs_runtime_state.initialized = true;

	return fs_error_code_t::OK;
}

/*
================
FS_Shutdown
================
*/
fs_error_code_t FS_Shutdown() {
	if ( !g_fs_runtime_state.initialized ) {
		return fs_error_code_t::ERR_NOT_INIT;
	}
	g_fs_runtime_state = {};
	g_fs_runtime_state.initialized = false;
	return fs_error_code_t::OK;
}

/*
================
FS_MountCount
================
*/
rcommon::u32 FS_MountCount() {
	return g_fs_runtime_state.mount_count;
}

/*
================
FS_IsInitialized
================
*/
bool FS_IsInitialized() {
	return g_fs_runtime_state.initialized;
}

/*
================
FS_MountDirectory

Adds a physical directory to the virtual search path list.
================
*/
fs_error_code_t FS_MountDirectory( const char *virtual_root, const char *physical_path, rcommon::u32 flags, rcommon::u32 priority ) {
	if ( !g_fs_runtime_state.initialized ) {
		return fs_error_code_t::ERR_NOT_INIT;
	}
	if ( virtual_root == nullptr ) {
		return fs_error_code_t::ERR_INVALID_PATH;
	}
	if ( physical_path == nullptr || physical_path[0] == '\0' ) {
		return fs_error_code_t::ERR_INVALID_PATH;
	}
	const rcommon::u32 allowed_flags = FS_MOUNT_READ_ONLY | FS_MOUNT_WRITABLE;
	if ( ( flags & ~allowed_flags ) != 0u ) {
		return fs_error_code_t::ERR_INVALID_ARGUMENT;
	}
	if ( ( flags & allowed_flags ) == 0u ) {
		return fs_error_code_t::ERR_INVALID_ARGUMENT;
	}

	if ( g_fs_runtime_state.mount_count >= FS_MAX_MOUNTS ) {
		return fs_error_code_t::ERR_TOO_MANY_MOUNTS;
	}

	fs_mount_t &mount = g_fs_runtime_state.mounts[g_fs_runtime_state.mount_count];

	mount.type = fs_mount_type_t::FS_DIRECTORY;

	std::strncpy( mount.virtual_root, virtual_root, sizeof( mount.virtual_root ) - 1u );
	mount.virtual_root[sizeof( mount.virtual_root ) - 1u] = '\0';

	std::strncpy( mount.physical_root, physical_path, sizeof( mount.physical_root ) - 1u );
	mount.physical_root[sizeof( mount.physical_root ) - 1u] = '\0';
	mount.flags = flags;
	mount.priority = priority;
	++g_fs_runtime_state.mount_count;
	return fs_error_code_t::OK;
}

/*
================
FS_UnmountDirectory
================
*/
fs_error_code_t FS_UnmountDirectory( const char *virtual_root ) {
	if ( !g_fs_runtime_state.initialized ) {
		return fs_error_code_t::ERR_NOT_INIT;
	}
	if ( virtual_root == nullptr ) {
		return fs_error_code_t::ERR_INVALID_PATH;
	}
	for ( rcommon::u32 i = 0u; i < g_fs_runtime_state.mount_count; ++i ) {
		if ( std::strcmp( g_fs_runtime_state.mounts[i].virtual_root, virtual_root ) == 0 ) {
			for ( rcommon::u32 j = i; j + 1u < g_fs_runtime_state.mount_count; ++j ) {
				g_fs_runtime_state.mounts[j] = g_fs_runtime_state.mounts[j + 1u];
			}
			--g_fs_runtime_state.mount_count;
			g_fs_runtime_state.mounts[g_fs_runtime_state.mount_count] = {};

			return fs_error_code_t::OK;
		}
	}
	return fs_error_code_t::ERR_MOUNT_NOT_FOUND;
}

/*
================
FS_SetWritePath
================
*/
fs_error_code_t FS_SetWritePath( const char *physical_path ) {
	if ( !g_fs_runtime_state.initialized ) {
		return fs_error_code_t::ERR_NOT_INIT;
	}
	if ( physical_path == nullptr || physical_path[0] == '\0' ) {
		return fs_error_code_t::ERR_INVALID_PATH;
	}
	std::strncpy(
		g_fs_runtime_state.write_path,
		physical_path,
		sizeof( g_fs_runtime_state.write_path ) - 1u );
	g_fs_runtime_state.write_path[sizeof( g_fs_runtime_state.write_path ) - 1u] = '\0';
	return fs_error_code_t::OK;
}

/*
================
FS_GetWritePath
================
*/
const char *FS_GetWritePath() {
	if ( !g_fs_runtime_state.initialized ) {
		return nullptr;
	}
	if ( g_fs_runtime_state.write_path[0] == '\0' ) {
		return nullptr;
	}
	return g_fs_runtime_state.write_path;
}

/*
================
FS_ResolvePath

Finds the first mounted physical path matching a virtual engine path.
================
*/
fs_error_code_t FS_ResolvePath( const char *virtual_path, char *out_resolved_path, rcommon::u32 out_resolved_path_size ) {
	if ( !g_fs_runtime_state.initialized ) {
		return fs_error_code_t::ERR_NOT_INIT;
	}
	if ( virtual_path == nullptr || virtual_path[0] == '\0' ) {
		return fs_error_code_t::ERR_INVALID_PATH;
	}
	if ( out_resolved_path == nullptr || out_resolved_path_size == 0u ) {
		return fs_error_code_t::ERR_INVALID_ARGUMENT;
	}
	out_resolved_path[0] = '\0';
	for ( rcommon::u32 i = 0u; i < g_fs_runtime_state.mount_count; ++i ) {
		const fs_mount_t &mount = g_fs_runtime_state.mounts[i];
		// Package backends will plug in here later.
		if ( mount.type != fs_mount_type_t::FS_DIRECTORY ) {
			continue;
		}
		char candidate_path[FS_MAX_PATH_LENGTH]{};

		if ( mount.virtual_root[0] == '\0' ) {
			const int written = std::snprintf( candidate_path, sizeof( candidate_path ), "%s/%s", mount.physical_root, virtual_path );

			if ( written < 0 || static_cast<rcommon::u32>( written ) >= sizeof( candidate_path ) ) {
				out_resolved_path[0] = '\0';
				return fs_error_code_t::ERR_BUFFER_TOO_SMALL;
			}
		} else {
			const rcommon::u32 virtual_root_len = static_cast<rcommon::u32>( std::strlen( mount.virtual_root ) );
			if ( std::strncmp( virtual_path, mount.virtual_root, virtual_root_len ) != 0 ) {
				continue;
			}
			const char next_char = virtual_path[virtual_root_len];
			if ( next_char != '\0' && next_char != '/' ) {
				continue;
			}
			const char *relative_path = virtual_path + virtual_root_len;
			if ( relative_path[0] == '/' ) {
				++relative_path;
			}
			const int written = std::snprintf( candidate_path, sizeof( candidate_path ), "%s/%s", mount.physical_root, relative_path );

			if ( written < 0 || static_cast<rcommon::u32>( written ) >= sizeof( candidate_path ) ) {
				out_resolved_path[0] = '\0';
				return fs_error_code_t::ERR_BUFFER_TOO_SMALL;
			}
		}

		std::error_code ec{};
		if ( !std::filesystem::exists( candidate_path, ec ) ) {
			continue;
		}
		if ( ec ) {
			return fs_error_code_t::ERR_IO_ERROR;
		}

		std::strncpy( out_resolved_path, candidate_path, out_resolved_path_size - 1u );
		out_resolved_path[out_resolved_path_size - 1u] = '\0';
		return fs_error_code_t::OK;
	}
	return fs_error_code_t::ERR_PATH_NOT_FOUND;
}

/*
================
FS_Exists
================
*/
bool FS_Exists( const char *virtual_path ) {
	if ( !g_fs_runtime_state.initialized ) {
		return false;
	}
	char resolved_path[FS_MAX_PATH_LENGTH]{};

	fs_error_code_t err = FS_ResolvePath( virtual_path, resolved_path, sizeof( resolved_path ) );

	return err == fs_error_code_t::OK;
}

/*
================
FS_GetFileInfo
================
*/
fs_error_code_t FS_GetFileInfo( const char *virtual_path, fs_file_info_t &out_info ) {
	if ( !g_fs_runtime_state.initialized ) {
		return fs_error_code_t::ERR_NOT_INIT;
	}
	out_info = {};
	if ( virtual_path == nullptr || virtual_path[0] == '\0' ) {
		return fs_error_code_t::ERR_INVALID_PATH;
	}
	fs_error_code_t err = FS_ResolvePath( virtual_path, out_info.resolved_path, sizeof( out_info.resolved_path ) );
	if ( err != fs_error_code_t::OK ) {
		return err;
	}
	std::error_code ec{};
	out_info.exists = std::filesystem::exists( out_info.resolved_path, ec );
	if ( ec ) {
		out_info = {};
		return fs_error_code_t::ERR_IO_ERROR;
	}
	out_info.is_directory = std::filesystem::is_directory( out_info.resolved_path, ec );
	if ( ec ) {
		out_info = {};
		return fs_error_code_t::ERR_IO_ERROR;
	}
	if ( !out_info.is_directory ) {
		out_info.file_size = static_cast<rcommon::u64>( std::filesystem::file_size( out_info.resolved_path, ec ) );

		if ( ec ) {
			out_info = {};
			return fs_error_code_t::ERR_IO_ERROR;
		}
	}
	return fs_error_code_t::OK;
}

/*
================
FS_OpenModeToCMode
================
*/
static const char *FS_OpenModeToCMode( const fs_open_mode_t mode ) {
	switch ( mode ) {
	case fs_open_mode_t::READ_TEXT:
		return "r";
	case fs_open_mode_t::READ_BINARY:
		return "rb";
	case fs_open_mode_t::WRITE_TEXT:
		return "w";
	case fs_open_mode_t::WRITE_BINARY:
		return "wb";
	case fs_open_mode_t::APPEND_TEXT:
		return "a";
	case fs_open_mode_t::APPEND_BINARY:
		return "ab";
	default:
		return nullptr;
	}
}

/*
================
FS_Open

Opens an OS-backed file resolved through the virtual filesystem.
================
*/
fs_error_code_t FS_Open( const char *virtual_path, fs_open_mode_t mode, fs_file_t &file ) {
	if ( !g_fs_runtime_state.initialized ) {
		return fs_error_code_t::ERR_NOT_INIT;
	}
	file = {};
	if ( virtual_path == nullptr || virtual_path[0] == '\0' ) {
		return fs_error_code_t::ERR_INVALID_PATH;
	}
	const char *c_mode = FS_OpenModeToCMode( mode );
	if ( c_mode == nullptr ) {
		return fs_error_code_t::ERR_INVALID_MODE;
	}
	const bool read_mode = mode == fs_open_mode_t::READ_TEXT || mode == fs_open_mode_t::READ_BINARY;
	const bool write_mode = mode == fs_open_mode_t::WRITE_TEXT || mode == fs_open_mode_t::WRITE_BINARY;
	const bool append_mode = mode == fs_open_mode_t::APPEND_TEXT || mode == fs_open_mode_t::APPEND_BINARY;
	char resolved_path[FS_MAX_PATH_LENGTH]{};
	if ( read_mode ) {
		const fs_error_code_t err = FS_ResolvePath( virtual_path, resolved_path, sizeof( resolved_path ) );
		if ( err != fs_error_code_t::OK ) {
			return err;
		}
	} else if ( write_mode || append_mode ) {
		if ( g_fs_runtime_state.write_path[0] == '\0' ) {
			return fs_error_code_t::ERR_PERMISSION_DENIED;
		}
		const int written = std::snprintf( resolved_path, sizeof( resolved_path ), "%s/%s", g_fs_runtime_state.write_path, virtual_path );
		if ( written < 0 || static_cast<rcommon::u32>( written ) >= sizeof( resolved_path ) ) {
			return fs_error_code_t::ERR_BUFFER_TOO_SMALL;
		}
		std::error_code ec{};
		const std::filesystem::path parent_path = std::filesystem::path( resolved_path ).parent_path();
		if ( !parent_path.empty() ) {
			std::filesystem::create_directories( parent_path, ec );
			if ( ec ) {
				return fs_error_code_t::ERR_IO_ERROR;
			}
		}
	}
	std::FILE *native_file = std::fopen( resolved_path, c_mode );
	if ( native_file == nullptr ) {
		return fs_error_code_t::ERR_FILE_OPEN_FAILED;
	}
	file.backend = fs_file_backend_t::OS_FILE;
	file.native_handle = native_file;
	file.readable = read_mode;
	file.writable = write_mode || append_mode;
	file.cursor = 0u;
	file.size = 0u;

	std::error_code ec{};

	if ( std::filesystem::exists( resolved_path, ec ) && !std::filesystem::is_directory( resolved_path, ec ) ) {
		file.size = static_cast<rcommon::u64>( std::filesystem::file_size( resolved_path, ec ) );

		if ( ec ) {
			std::fclose( native_file );
			file = {};
			return fs_error_code_t::ERR_IO_ERROR;
		}
	}
	if ( append_mode ) {
		file.cursor = file.size;
	}
	return fs_error_code_t::OK;
}

/*
================
FS_Close
================
*/
fs_error_code_t FS_Close( fs_file_t &file ) {
	if ( !g_fs_runtime_state.initialized ) {
		return fs_error_code_t::ERR_NOT_INIT;
	}

	if ( file.backend != fs_file_backend_t::OS_FILE ) {
		return fs_error_code_t::ERR_UNSUPPORTED_BACKEND;
	}

	if ( file.native_handle == nullptr ) {
		return fs_error_code_t::ERR_INVALID_HANDLE;
	}

	std::FILE *native_file = static_cast<std::FILE *>( file.native_handle );

	if ( std::fclose( native_file ) != 0 ) {
		file = {};
		return fs_error_code_t::ERR_FILE_CLOSE_FAILED;
	}

	file = {};

	return fs_error_code_t::OK;
}

/*
================
FS_Read
================
*/
fs_error_code_t FS_Read( fs_file_t &file, void *buffer, rcommon::u64 bytes_to_read, rcommon::u64 &bytes_read_out ) {
	bytes_read_out = 0u;

	if ( !g_fs_runtime_state.initialized ) {
		return fs_error_code_t::ERR_NOT_INIT;
	}

	if ( file.backend != fs_file_backend_t::OS_FILE ) {
		return fs_error_code_t::ERR_UNSUPPORTED_BACKEND;
	}

	if ( file.native_handle == nullptr ) {
		return fs_error_code_t::ERR_INVALID_HANDLE;
	}

	if ( !file.readable ) {
		return fs_error_code_t::ERR_PERMISSION_DENIED;
	}

	if ( bytes_to_read == 0u ) {
		return fs_error_code_t::OK;
	}

	if ( buffer == nullptr ) {
		return fs_error_code_t::ERR_INVALID_ARGUMENT;
	}

	const std::size_t read_size = static_cast<std::size_t>( bytes_to_read );

	if ( static_cast<rcommon::u64>( read_size ) != bytes_to_read ) {
		return fs_error_code_t::ERR_INVALID_ARGUMENT;
	}

	std::FILE *native_file = static_cast<std::FILE *>( file.native_handle );
	const std::size_t bytes_read = std::fread( buffer, 1u, read_size, native_file );

	bytes_read_out = static_cast<rcommon::u64>( bytes_read );
	file.cursor += bytes_read_out;

	if ( bytes_read != read_size && std::ferror( native_file ) != 0 ) {
		return fs_error_code_t::ERR_FILE_READ_FAILED;
	}

	return fs_error_code_t::OK;
}

/*
================
FS_Write
================
*/
fs_error_code_t FS_Write( fs_file_t &file, const void *buffer, rcommon::u64 bytes_to_write, rcommon::u64 &bytes_written_out ) {
	bytes_written_out = 0u;

	if ( !g_fs_runtime_state.initialized ) {
		return fs_error_code_t::ERR_NOT_INIT;
	}
	if ( file.backend != fs_file_backend_t::OS_FILE ) {
		return fs_error_code_t::ERR_UNSUPPORTED_BACKEND;
	}
	if ( file.native_handle == nullptr ) {
		return fs_error_code_t::ERR_INVALID_HANDLE;
	}
	if ( !file.writable ) {
		return fs_error_code_t::ERR_PERMISSION_DENIED;
	}
	if ( bytes_to_write == 0u ) {
		return fs_error_code_t::OK;
	}
	if ( buffer == nullptr ) {
		return fs_error_code_t::ERR_INVALID_ARGUMENT;
	}
	const std::size_t write_size = static_cast<std::size_t>( bytes_to_write );

	if ( static_cast<rcommon::u64>( write_size ) != bytes_to_write ) {
		return fs_error_code_t::ERR_INVALID_ARGUMENT;
	}
	std::FILE *native_file = static_cast<std::FILE *>( file.native_handle );
	const std::size_t bytes_written = std::fwrite( buffer, 1u, write_size, native_file );

	bytes_written_out = static_cast<rcommon::u64>( bytes_written );
	file.cursor += bytes_written_out;

	if ( file.cursor > file.size ) {
		file.size = file.cursor;
	}

	if ( bytes_written != write_size ) {
		return fs_error_code_t::ERR_FILE_WRITE_FAILED;
	}

	return fs_error_code_t::OK;
}

/*
================
FS_Seek
================
*/
fs_error_code_t FS_Seek( fs_file_t &file, rcommon::i64 offset, fs_seek_origin_t origin ) {
	if ( !g_fs_runtime_state.initialized ) {
		return fs_error_code_t::ERR_NOT_INIT;
	}
	if ( file.backend != fs_file_backend_t::OS_FILE ) {
		return fs_error_code_t::ERR_UNSUPPORTED_BACKEND;
	}

	if ( file.native_handle == nullptr ) {
		return fs_error_code_t::ERR_INVALID_HANDLE;
	}

	int c_origin{};

	switch ( origin ) {
	case fs_seek_origin_t::FS_SEEK_START:
		c_origin = SEEK_SET;
		break;
	case fs_seek_origin_t::FS_SEEK_CURRENT:
		c_origin = SEEK_CUR;
		break;
	case fs_seek_origin_t::FS_SEEK_END:
		c_origin = SEEK_END;
		break;
	default:
		return fs_error_code_t::ERR_INVALID_ARGUMENT;
	}

	std::FILE *native_file = static_cast<std::FILE *>( file.native_handle );

	if ( std::fseek( native_file, static_cast<long>( offset ), c_origin ) != 0 ) {
		return fs_error_code_t::ERR_FILE_SEEK_FAILED;
	}

	const long position = std::ftell( native_file );

	if ( position < 0 ) {
		return fs_error_code_t::ERR_FILE_TELL_FAILED;
	}

	file.cursor = static_cast<rcommon::u64>( position );

	return fs_error_code_t::OK;
}

/*
================
FS_Tell
================
*/
fs_error_code_t FS_Tell( fs_file_t &file, rcommon::u64 &out_position ) {
	out_position = 0u;

	if ( !g_fs_runtime_state.initialized ) {
		return fs_error_code_t::ERR_NOT_INIT;
	}

	if ( file.backend != fs_file_backend_t::OS_FILE ) {
		return fs_error_code_t::ERR_UNSUPPORTED_BACKEND;
	}

	if ( file.native_handle == nullptr ) {
		return fs_error_code_t::ERR_INVALID_HANDLE;
	}

	std::FILE *native_file = static_cast<std::FILE *>( file.native_handle );
	const long position = std::ftell( native_file );

	if ( position < 0 ) {
		return fs_error_code_t::ERR_FILE_TELL_FAILED;
	}

	out_position = static_cast<rcommon::u64>( position );
	file.cursor = out_position;

	return fs_error_code_t::OK;
}

/*
================
FS_ReadEntireFile
================
*/
fs_error_code_t FS_ReadEntireFile( const char *virtual_path, void *buffer, rcommon::u64 bytes_to_read, rcommon::u64 &bytes_read_out ) {
	bytes_read_out = 0u;

	if ( !g_fs_runtime_state.initialized ) {
		return fs_error_code_t::ERR_NOT_INIT;
	}

	if ( buffer == nullptr ) {
		return fs_error_code_t::ERR_INVALID_ARGUMENT;
	}

	fs_file_t file{};
	fs_error_code_t err = FS_Open( virtual_path, fs_open_mode_t::READ_BINARY, file );

	if ( err != fs_error_code_t::OK ) {
		return err;
	}

	if ( file.size > bytes_to_read ) {
		FS_Close( file );
		return fs_error_code_t::ERR_BUFFER_TOO_SMALL;
	}

	if ( file.size == 0u ) {
		return FS_Close( file );
	}

	const rcommon::u64 expected_size = file.size;
	err = FS_Read( file, buffer, expected_size, bytes_read_out );
	const fs_error_code_t close_err = FS_Close( file );

	if ( err != fs_error_code_t::OK ) {
		return err;
	}

	if ( close_err != fs_error_code_t::OK ) {
		return close_err;
	}

	if ( bytes_read_out != expected_size ) {
		return fs_error_code_t::ERR_FILE_READ_FAILED;
	}

	return fs_error_code_t::OK;
}

} // namespace reap::rengine::fs

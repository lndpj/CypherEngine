/*======================================================================
   File: CypherFileSystem.cpp
   Project: CypherEngine
   Author: ksiric <email@example.com>
   Created: 2026-04-26 15:53:16
   Last Modified by: ksiric
   Last Modified: 2026-06-12 10:09:04
   ---------------------------------------------------------------------
   Description:

   ---------------------------------------------------------------------
   License:
   Company:
   Version: 0.1.0
 ======================================================================
																	   */

#include "CypherEngine/CypherFileSystem/CypherFileSystem.h"
#include "CypherEngine/CypherCommon/CypherCommon_Print.h"
#include "CypherEngine/CypherLog/CypherLog.h"

#include <cstdio>           // stdio file handles and read/write operations.
#include <cstring>          // strcmp / strncpy for fixed path buffers.
#include <filesystem>       // Path probing and directory creation.
#include <system_error>     // std::error_code for non-throwing filesystem calls.
#include <cctype>           // isalpha and etc...

namespace cypher::engine::fs {

namespace {
/*
================
Filesystem Runtime State
================
*/

struct runtime_state_t {
	bool initialized{ false };
	mount_t mounts[CYPHER_FILESYSTEM_MAX_MOUNTS]{};
	common::u32 mount_count{ 0u };
	char write_path[CYPHER_FILESYSTEM_MAX_PATH_LENGTH]{};
};

runtime_state_t g_fs_runtime_state{};

} // namespace

/*
================
CypherFileSystem_Init
================
*/
error_code_t CypherFileSystem_Init() {
	if ( g_fs_runtime_state.initialized ) {
		LOG_WARNING( log::channel_t::FS, "filesystem init requested while already initialized." );
		return error_code_t::ERR_IS_INIT;
	}

	g_fs_runtime_state = {};
	g_fs_runtime_state.initialized = true;

	LOG_INFO( log::channel_t::FS, "filesystem initialized." );

	return error_code_t::OK;
}

/*
================
CypherFileSystem_Shutdown
================
*/
error_code_t CypherFileSystem_Shutdown() {
	if ( !g_fs_runtime_state.initialized ) {
		LOG_WARNING( log::channel_t::FS, "filesystem shutdown requested while not initialized." );
		return error_code_t::ERR_NOT_INIT;
	}
	LOG_INFO( log::channel_t::FS, "filesystem shutdown: mounts=%u.", g_fs_runtime_state.mount_count );
	g_fs_runtime_state = {};
	g_fs_runtime_state.initialized = false;
	return error_code_t::OK;
}

/*
================
CypherFileSystem_MountCount
================
*/
common::u32 CypherFileSystem_MountCount() {
	return g_fs_runtime_state.mount_count;
}

/*
================
CypherFileSystem_IsInitialized
================
*/
bool CypherFileSystem_IsInitialized() {
	return g_fs_runtime_state.initialized;
}

/*
================
CypherFileSystem_MountDirectory

Adds a physical directory to the virtual search path list.
================
*/
error_code_t CypherFileSystem_MountDirectory( const char *virtual_root, const char *physical_path, common::u32 flags, common::u32 priority ) {
	if ( !g_fs_runtime_state.initialized ) {
		LOG_ERROR( log::channel_t::FS, "mount failed: filesystem is not initialized." );
		return error_code_t::ERR_NOT_INIT;
	}
	if ( virtual_root == nullptr ) {
		LOG_ERROR( log::channel_t::FS, "mount failed: virtual root is null." );
		return error_code_t::ERR_INVALID_PATH;
	}
	if ( physical_path == nullptr || physical_path[0] == '\0' ) {
		LOG_ERROR( log::channel_t::FS, "mount failed: physical path is invalid." );
		return error_code_t::ERR_INVALID_PATH;
	}
	const common::u32 allowed_flags = CYPHER_FILESYSTEM_MOUNT_READ_ONLY | CYPHER_FILESYSTEM_MOUNT_WRITABLE;
	if ( ( flags & ~allowed_flags ) != 0u ) {
		LOG_ERROR( log::channel_t::FS, "mount failed for '%s': invalid flags 0x%x.", physical_path, flags );
		return error_code_t::ERR_INVALID_ARGUMENT;
	}
	if ( ( flags & allowed_flags ) == 0u ) {
		LOG_ERROR( log::channel_t::FS, "mount failed for '%s': no mount access flags set.", physical_path );
		return error_code_t::ERR_INVALID_ARGUMENT;
	}

	if ( g_fs_runtime_state.mount_count >= CYPHER_FILESYSTEM_MAX_MOUNTS ) {
		LOG_ERROR( log::channel_t::FS, "mount failed for '%s': mount table full (%u).", physical_path, CYPHER_FILESYSTEM_MAX_MOUNTS );
		return error_code_t::ERR_TOO_MANY_MOUNTS;
	}

	mount_t &mount = g_fs_runtime_state.mounts[g_fs_runtime_state.mount_count];

	mount.type = mount_type_t::CYPHER_FILESYSTEM_DIRECTORY;

	std::strncpy( mount.virtual_root, virtual_root, sizeof( mount.virtual_root ) - 1u );
	mount.virtual_root[sizeof( mount.virtual_root ) - 1u] = '\0';

	std::strncpy( mount.physical_root, physical_path, sizeof( mount.physical_root ) - 1u );
	mount.physical_root[sizeof( mount.physical_root ) - 1u] = '\0';
	mount.flags = flags;
	mount.priority = priority;
	++g_fs_runtime_state.mount_count;
	LOG_INFO( log::channel_t::FS, "mounted '%s' -> '%s' flags=0x%x priority=%u.", virtual_root[0] ? virtual_root : "<root>", physical_path, flags, priority );
	return error_code_t::OK;
}

/*
================
CypherFileSystem_UnmountDirectory
================
*/
error_code_t CypherFileSystem_UnmountDirectory( const char *virtual_root ) {
	if ( !g_fs_runtime_state.initialized ) {
		return error_code_t::ERR_NOT_INIT;
	}
	if ( virtual_root == nullptr ) {
		return error_code_t::ERR_INVALID_PATH;
	}
	for ( common::u32 i = 0u; i < g_fs_runtime_state.mount_count; ++i ) {
		if ( std::strcmp( g_fs_runtime_state.mounts[i].virtual_root, virtual_root ) == 0 ) {
			for ( common::u32 j = i; j + 1u < g_fs_runtime_state.mount_count; ++j ) {
				g_fs_runtime_state.mounts[j] = g_fs_runtime_state.mounts[j + 1u];
			}
			--g_fs_runtime_state.mount_count;
			g_fs_runtime_state.mounts[g_fs_runtime_state.mount_count] = {};

			return error_code_t::OK;
		}
	}
	return error_code_t::ERR_MOUNT_NOT_FOUND;
}

/*
================
CypherFileSystem_SetWritePath
================
*/
error_code_t CypherFileSystem_SetWritePath( const char *physical_path ) {
	if ( !g_fs_runtime_state.initialized ) {
		LOG_ERROR( log::channel_t::FS, "set write path failed: filesystem is not initialized." );
		return error_code_t::ERR_NOT_INIT;
	}
	if ( physical_path == nullptr || physical_path[0] == '\0' ) {
		LOG_ERROR( log::channel_t::FS, "set write path failed: invalid physical path." );
		return error_code_t::ERR_INVALID_PATH;
	}
	std::strncpy(
		g_fs_runtime_state.write_path,
		physical_path,
		sizeof( g_fs_runtime_state.write_path ) - 1u );
	g_fs_runtime_state.write_path[sizeof( g_fs_runtime_state.write_path ) - 1u] = '\0';
	LOG_INFO( log::channel_t::FS, "write path set to '%s'.", g_fs_runtime_state.write_path );
	return error_code_t::OK;
}

/*
================
CypherFileSystem_GetWritePath
================
*/
const char *CypherFileSystem_GetWritePath() {
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
CypherFileSystem_ResolvePath

Finds the first mounted physical path matching a virtual engine path.
================
*/
error_code_t CypherFileSystem_ResolvePath( const char *virtual_path, char *out_resolved_path, common::u32 out_resolved_path_size ) {
	if ( !g_fs_runtime_state.initialized ) {
		return error_code_t::ERR_NOT_INIT;
	}
	if ( virtual_path == nullptr || virtual_path[0] == '\0' ) {
		return error_code_t::ERR_INVALID_PATH;
	}
	if ( out_resolved_path == nullptr || out_resolved_path_size == 0u ) {
		return error_code_t::ERR_INVALID_ARGUMENT;
	}
	out_resolved_path[0] = '\0';
    // @NormalizationOfPaths -> Added normalization of paths for better safety of virtual pathing.
    char normalized_path[CYPHER_FILESYSTEM_MAX_PATH_LENGTH]{};
    error_code_t normalize_result = CypherFileSystem_NormalizeVirtualPath( virtual_path, normalized_path, sizeof( normalized_path ) );
    if ( normalize_result != error_code_t::OK ) {
        return normalize_result;
    }
	for ( common::u32 i = 0u; i < g_fs_runtime_state.mount_count; ++i ) {
		const mount_t &mount = g_fs_runtime_state.mounts[i];
		// Package backends will plug in here later.
		if ( mount.type != mount_type_t::CYPHER_FILESYSTEM_DIRECTORY ) {
			continue;
		}
		char candidate_path[CYPHER_FILESYSTEM_MAX_PATH_LENGTH]{};

		if ( mount.virtual_root[0] == '\0' ) {
			const int written = std::snprintf( candidate_path, sizeof( candidate_path ), "%s/%s", mount.physical_root, normalized_path );

			if ( written < 0 || static_cast<common::u32>( written ) >= sizeof( candidate_path ) ) {
				out_resolved_path[0] = '\0';
				return error_code_t::ERR_BUFFER_TOO_SMALL;
			}
		} else {
			const common::u32 virtual_root_len = static_cast<common::u32>( std::strlen( mount.virtual_root ) );
			if ( std::strncmp( normalized_path, mount.virtual_root, virtual_root_len ) != 0 ) {
				continue;
			}
			const char next_char = normalized_path[virtual_root_len];
			if ( next_char != '\0' && next_char != '/' ) {
				continue;
			}
			const char *relative_path = normalized_path + virtual_root_len;
			if ( relative_path[0] == '/' ) {
				++relative_path;
			}
			const int written = std::snprintf( candidate_path, sizeof( candidate_path ), "%s/%s", mount.physical_root, relative_path );

			if ( written < 0 || static_cast<common::u32>( written ) >= sizeof( candidate_path ) ) {
				out_resolved_path[0] = '\0';
				return error_code_t::ERR_BUFFER_TOO_SMALL;
			}
		}

		std::error_code ec{};
		if ( !std::filesystem::exists( candidate_path, ec ) ) {
			continue;
		}
		if ( ec ) {
			return error_code_t::ERR_IO_ERROR;
		}

		std::strncpy( out_resolved_path, candidate_path, out_resolved_path_size - 1u );
		out_resolved_path[out_resolved_path_size - 1u] = '\0';
		return error_code_t::OK;
	}
	return error_code_t::ERR_PATH_NOT_FOUND;
}

/*
================
CypherFileSystem_NormalizeVirtualPath
================
*/
error_code_t CypherFileSystem_NormalizeVirtualPath( const char *virtual_path, char *out_path, common::u32 out_path_size )
{
    if ( virtual_path == nullptr || virtual_path[0] == '\0' ) {
        return error_code_t::ERR_INVALID_PATH;
    }
    if ( out_path == nullptr || out_path_size == 0u ) {
        return error_code_t::ERR_INVALID_ARGUMENT;
    }
    out_path[0] = '\0';

    if ( virtual_path[0] == '/' || virtual_path[0] == '\\' ) {
        return error_code_t::ERR_INVALID_PATH;
    }

    if ( std::isalpha(static_cast<unsigned char>( virtual_path[0] ) ) && virtual_path[1] == ':' ) {
        return error_code_t::ERR_INVALID_PATH;
    }
    const char *cursor = virtual_path;

    common::u32 write_index = 0u;

    while ( *cursor != '\0' ) {
        // skip the characters for slashes
        while ( *cursor == '/' || *cursor == '\\' ) {
            ++cursor;
        }
        const char *segment_start = cursor;
        while ( *cursor != '/' && *cursor != '\\' && *cursor != '\0' ) {
            ++cursor;
        }
        const char *segment_end = cursor;

        const common::u32 segment_length = static_cast<common::u32>( segment_end - segment_start );
        if ( segment_length == 0u ) {
            continue;
        }
        if ( segment_length == 1u && segment_start[0] == '.' ) {
            continue;
        }
        if ( segment_length == 2u && segment_start[0] == '.' && segment_start[1] == '.' ) {
            out_path[0] = '\0';
            return error_code_t::ERR_INVALID_PATH;
        }
        if ( write_index != 0u ) {
            if ( write_index + 1u >= out_path_size ) {
                out_path[0] = '\0';
                return error_code_t::ERR_BUFFER_TOO_SMALL;
            }

            out_path[write_index] = '/';
            ++write_index;
        }

        for ( common::u32 i = 0u; i < segment_length; ++i ) {
            if ( write_index + 1u >= out_path_size ) {
                out_path[0] = '\0';
                return error_code_t::ERR_BUFFER_TOO_SMALL;
            }
            out_path[write_index] = segment_start[i];
            ++write_index;
        }

        out_path[write_index] = '\0';
    }

    if ( write_index == 0u ) {
        return error_code_t::ERR_INVALID_PATH;
    }

    return error_code_t::OK;
}

/*
================
CypherFileSystem_Exists
================
*/
bool CypherFileSystem_Exists( const char *virtual_path ) {
	if ( !g_fs_runtime_state.initialized ) {
		return false;
	}
	char resolved_path[CYPHER_FILESYSTEM_MAX_PATH_LENGTH]{};

	error_code_t err = CypherFileSystem_ResolvePath( virtual_path, resolved_path, sizeof( resolved_path ) );

	return err == error_code_t::OK;
}

/*
================
CypherFileSystem_GetFileInfo
================
*/
error_code_t CypherFileSystem_GetFileInfo( const char *virtual_path, file_info_t &out_info ) {
	if ( !g_fs_runtime_state.initialized ) {
		return error_code_t::ERR_NOT_INIT;
	}
	out_info = {};
	if ( virtual_path == nullptr || virtual_path[0] == '\0' ) {
		return error_code_t::ERR_INVALID_PATH;
	}
	error_code_t err = CypherFileSystem_ResolvePath( virtual_path, out_info.resolved_path, sizeof( out_info.resolved_path ) );
	if ( err != error_code_t::OK ) {
		return err;
	}
	std::error_code ec{};
	out_info.exists = std::filesystem::exists( out_info.resolved_path, ec );
	if ( ec ) {
		out_info = {};
		return error_code_t::ERR_IO_ERROR;
	}
	out_info.is_directory = std::filesystem::is_directory( out_info.resolved_path, ec );
	if ( ec ) {
		out_info = {};
		return error_code_t::ERR_IO_ERROR;
	}
	if ( !out_info.is_directory ) {
		out_info.file_size = static_cast<common::u64>( std::filesystem::file_size( out_info.resolved_path, ec ) );

		if ( ec ) {
			out_info = {};
			return error_code_t::ERR_IO_ERROR;
		}
	}
	return error_code_t::OK;
}

/*
================
CypherFileSystem_OpenModeToCMode
================
*/
static const char *CypherFileSystem_OpenModeToCMode( const open_mode_t mode ) {
	switch ( mode ) {
	case open_mode_t::READ_TEXT:
		return "r";
	case open_mode_t::READ_BINARY:
		return "rb";
	case open_mode_t::WRITE_TEXT:
		return "w";
	case open_mode_t::WRITE_BINARY:
		return "wb";
	case open_mode_t::APPEND_TEXT:
		return "a";
	case open_mode_t::APPEND_BINARY:
		return "ab";
	default:
		return nullptr;
	}
}

/*
================
CypherFileSystem_Open

Opens an OS-backed file resolved through the virtual filesystem.
================
*/
error_code_t CypherFileSystem_Open( const char *virtual_path, open_mode_t mode, file_t &file ) {
	if ( !g_fs_runtime_state.initialized ) {
		return error_code_t::ERR_NOT_INIT;
	}
	file = {};
	if ( virtual_path == nullptr || virtual_path[0] == '\0' ) {
		return error_code_t::ERR_INVALID_PATH;
	}
	const char *c_mode = CypherFileSystem_OpenModeToCMode( mode );
	if ( c_mode == nullptr ) {
		return error_code_t::ERR_INVALID_MODE;
	}
	const bool read_mode = mode == open_mode_t::READ_TEXT || mode == open_mode_t::READ_BINARY;
	const bool write_mode = mode == open_mode_t::WRITE_TEXT || mode == open_mode_t::WRITE_BINARY;
	const bool append_mode = mode == open_mode_t::APPEND_TEXT || mode == open_mode_t::APPEND_BINARY;
	char resolved_path[CYPHER_FILESYSTEM_MAX_PATH_LENGTH]{};
	if ( read_mode ) {
		const error_code_t err = CypherFileSystem_ResolvePath( virtual_path, resolved_path, sizeof( resolved_path ) );
		if ( err != error_code_t::OK ) {
			return err;
		}
	} else if ( write_mode || append_mode ) {
		if ( g_fs_runtime_state.write_path[0] == '\0' ) {
			return error_code_t::ERR_PERMISSION_DENIED;
		}
		char normalized_path[CYPHER_FILESYSTEM_MAX_PATH_LENGTH]{};
		const error_code_t normalize_result = CypherFileSystem_NormalizeVirtualPath( virtual_path, normalized_path, sizeof( normalized_path ) );
		if ( normalize_result != error_code_t::OK ) {
			return normalize_result;
		}
		const int written = std::snprintf( resolved_path, sizeof( resolved_path ), "%s/%s", g_fs_runtime_state.write_path, normalized_path );
		if ( written < 0 || static_cast<common::u32>( written ) >= sizeof( resolved_path ) ) {
			return error_code_t::ERR_BUFFER_TOO_SMALL;
		}
		std::error_code ec{};
		const std::filesystem::path parent_path = std::filesystem::path( resolved_path ).parent_path();
		if ( !parent_path.empty() ) {
			std::filesystem::create_directories( parent_path, ec );
			if ( ec ) {
				return error_code_t::ERR_IO_ERROR;
			}
		}
	}
	std::FILE *native_file = std::fopen( resolved_path, c_mode );
	if ( native_file == nullptr ) {
		return error_code_t::ERR_FILE_OPEN_FAILED;
	}
	file.backend = file_backend_t::OS_FILE;
	file.native_handle = native_file;
	file.readable = read_mode;
	file.writable = write_mode || append_mode;
	file.cursor = 0u;
	file.size = 0u;

	std::error_code ec{};

	if ( std::filesystem::exists( resolved_path, ec ) && !std::filesystem::is_directory( resolved_path, ec ) ) {
		file.size = static_cast<common::u64>( std::filesystem::file_size( resolved_path, ec ) );

		if ( ec ) {
			std::fclose( native_file );
			file = {};
			return error_code_t::ERR_IO_ERROR;
		}
	}
	if ( append_mode ) {
		file.cursor = file.size;
	}
	return error_code_t::OK;
}

/*
================
CypherFileSystem_Close
================
*/
error_code_t CypherFileSystem_Close( file_t &file ) {
	if ( !g_fs_runtime_state.initialized ) {
		return error_code_t::ERR_NOT_INIT;
	}

	if ( file.backend != file_backend_t::OS_FILE ) {
		return error_code_t::ERR_UNSUPPORTED_BACKEND;
	}

	if ( file.native_handle == nullptr ) {
		return error_code_t::ERR_INVALID_HANDLE;
	}

	std::FILE *native_file = static_cast<std::FILE *>( file.native_handle );

	if ( std::fclose( native_file ) != 0 ) {
		file = {};
		return error_code_t::ERR_FILE_CLOSE_FAILED;
	}

	file = {};

	return error_code_t::OK;
}

/*
================
CypherFileSystem_Read
================
*/
error_code_t CypherFileSystem_Read( file_t &file, void *buffer, common::u64 bytes_to_read, common::u64 &bytes_read_out ) {
	bytes_read_out = 0u;

	if ( !g_fs_runtime_state.initialized ) {
		return error_code_t::ERR_NOT_INIT;
	}

	if ( file.backend != file_backend_t::OS_FILE ) {
		return error_code_t::ERR_UNSUPPORTED_BACKEND;
	}

	if ( file.native_handle == nullptr ) {
		return error_code_t::ERR_INVALID_HANDLE;
	}

	if ( !file.readable ) {
		return error_code_t::ERR_PERMISSION_DENIED;
	}

	if ( bytes_to_read == 0u ) {
		return error_code_t::OK;
	}

	if ( buffer == nullptr ) {
		return error_code_t::ERR_INVALID_ARGUMENT;
	}

	const std::size_t read_size = static_cast<std::size_t>( bytes_to_read );

	if ( static_cast<common::u64>( read_size ) != bytes_to_read ) {
		return error_code_t::ERR_INVALID_ARGUMENT;
	}

	std::FILE *native_file = static_cast<std::FILE *>( file.native_handle );
	const std::size_t bytes_read = std::fread( buffer, 1u, read_size, native_file );

	bytes_read_out = static_cast<common::u64>( bytes_read );
	file.cursor += bytes_read_out;

	if ( bytes_read != read_size && std::ferror( native_file ) != 0 ) {
		return error_code_t::ERR_FILE_READ_FAILED;
	}

	return error_code_t::OK;
}

/*
================
CypherFileSystem_Write
================
*/
error_code_t CypherFileSystem_Write( file_t &file, const void *buffer, common::u64 bytes_to_write, common::u64 &bytes_written_out ) {
	bytes_written_out = 0u;

	if ( !g_fs_runtime_state.initialized ) {
		return error_code_t::ERR_NOT_INIT;
	}
	if ( file.backend != file_backend_t::OS_FILE ) {
		return error_code_t::ERR_UNSUPPORTED_BACKEND;
	}
	if ( file.native_handle == nullptr ) {
		return error_code_t::ERR_INVALID_HANDLE;
	}
	if ( !file.writable ) {
		return error_code_t::ERR_PERMISSION_DENIED;
	}
	if ( bytes_to_write == 0u ) {
		return error_code_t::OK;
	}
	if ( buffer == nullptr ) {
		return error_code_t::ERR_INVALID_ARGUMENT;
	}
	const std::size_t write_size = static_cast<std::size_t>( bytes_to_write );

	if ( static_cast<common::u64>( write_size ) != bytes_to_write ) {
		return error_code_t::ERR_INVALID_ARGUMENT;
	}
	std::FILE *native_file = static_cast<std::FILE *>( file.native_handle );
	const std::size_t bytes_written = std::fwrite( buffer, 1u, write_size, native_file );

	bytes_written_out = static_cast<common::u64>( bytes_written );
	file.cursor += bytes_written_out;

	if ( file.cursor > file.size ) {
		file.size = file.cursor;
	}

	if ( bytes_written != write_size ) {
		return error_code_t::ERR_FILE_WRITE_FAILED;
	}

	return error_code_t::OK;
}

/*
================
CypherFileSystem_Seek
================
*/
error_code_t CypherFileSystem_Seek( file_t &file, common::i64 offset, seek_origin_t origin ) {
	if ( !g_fs_runtime_state.initialized ) {
		return error_code_t::ERR_NOT_INIT;
	}
	if ( file.backend != file_backend_t::OS_FILE ) {
		return error_code_t::ERR_UNSUPPORTED_BACKEND;
	}

	if ( file.native_handle == nullptr ) {
		return error_code_t::ERR_INVALID_HANDLE;
	}

	int c_origin{};

	switch ( origin ) {
	case seek_origin_t::CYPHER_FILESYSTEM_SEEK_START:
		c_origin = SEEK_SET;
		break;
	case seek_origin_t::CYPHER_FILESYSTEM_SEEK_CURRENT:
		c_origin = SEEK_CUR;
		break;
	case seek_origin_t::CYPHER_FILESYSTEM_SEEK_END:
		c_origin = SEEK_END;
		break;
	default:
		return error_code_t::ERR_INVALID_ARGUMENT;
	}

	std::FILE *native_file = static_cast<std::FILE *>( file.native_handle );

	if ( std::fseek( native_file, static_cast<long>( offset ), c_origin ) != 0 ) {
		return error_code_t::ERR_FILE_SEEK_FAILED;
	}

	const long position = std::ftell( native_file );

	if ( position < 0 ) {
		return error_code_t::ERR_FILE_TELL_FAILED;
	}

	file.cursor = static_cast<common::u64>( position );

	return error_code_t::OK;
}

/*
================
CypherFileSystem_Tell
================
*/
error_code_t CypherFileSystem_Tell( file_t &file, common::u64 &out_position ) {
	out_position = 0u;

	if ( !g_fs_runtime_state.initialized ) {
		return error_code_t::ERR_NOT_INIT;
	}

	if ( file.backend != file_backend_t::OS_FILE ) {
		return error_code_t::ERR_UNSUPPORTED_BACKEND;
	}

	if ( file.native_handle == nullptr ) {
		return error_code_t::ERR_INVALID_HANDLE;
	}

	std::FILE *native_file = static_cast<std::FILE *>( file.native_handle );
	const long position = std::ftell( native_file );

	if ( position < 0 ) {
		return error_code_t::ERR_FILE_TELL_FAILED;
	}

	out_position = static_cast<common::u64>( position );
	file.cursor = out_position;

	return error_code_t::OK;
}

/*
================
CypherFileSystem_ReadEntireFile
================
*/
error_code_t CypherFileSystem_ReadEntireFile( const char *virtual_path, void *buffer, common::u64 bytes_to_read, common::u64 &bytes_read_out ) {
	bytes_read_out = 0u;

	if ( !g_fs_runtime_state.initialized ) {
		LOG_ERROR( log::channel_t::FS, "read entire file failed for '%s': filesystem is not initialized.", virtual_path ? virtual_path : "<null>" );
		return error_code_t::ERR_NOT_INIT;
	}

	if ( buffer == nullptr ) {
		LOG_ERROR( log::channel_t::FS, "read entire file failed for '%s': output buffer is null.", virtual_path ? virtual_path : "<null>" );
		return error_code_t::ERR_INVALID_ARGUMENT;
	}

	file_t file{};
	error_code_t err = CypherFileSystem_Open( virtual_path, open_mode_t::READ_BINARY, file );

	if ( err != error_code_t::OK ) {
		LOG_ERROR( log::channel_t::FS, "read entire file failed for '%s': open failed: %s.", virtual_path ? virtual_path : "<null>", CypherFileSystem_ErrorDesc( err ) );
		return err;
	}

	if ( file.size > bytes_to_read ) {
		CypherFileSystem_Close( file );
		LOG_ERROR( log::channel_t::FS, "read entire file failed for '%s': buffer too small, file=%llu bytes, buffer=%llu bytes.",
		                  virtual_path ? virtual_path : "<null>",
		                  static_cast<unsigned long long>( file.size ),
		                  static_cast<unsigned long long>( bytes_to_read ) );
		return error_code_t::ERR_BUFFER_TOO_SMALL;
	}

	if ( file.size == 0u ) {
		return CypherFileSystem_Close( file );
	}

	const common::u64 expected_size = file.size;
	err = CypherFileSystem_Read( file, buffer, expected_size, bytes_read_out );
	const error_code_t close_err = CypherFileSystem_Close( file );

	if ( err != error_code_t::OK ) {
		LOG_ERROR( log::channel_t::FS, "read entire file failed for '%s': read failed: %s.", virtual_path ? virtual_path : "<null>", CypherFileSystem_ErrorDesc( err ) );
		return err;
	}

	if ( close_err != error_code_t::OK ) {
		LOG_ERROR( log::channel_t::FS, "read entire file failed for '%s': close failed: %s.", virtual_path ? virtual_path : "<null>", CypherFileSystem_ErrorDesc( close_err ) );
		return close_err;
	}

	if ( bytes_read_out != expected_size ) {
		LOG_ERROR( log::channel_t::FS, "read entire file failed for '%s': short read, expected=%llu, actual=%llu.",
		                  virtual_path ? virtual_path : "<null>",
		                  static_cast<unsigned long long>( expected_size ),
		                  static_cast<unsigned long long>( bytes_read_out ) );
		return error_code_t::ERR_FILE_READ_FAILED;
	}

	return error_code_t::OK;
}

} // namespace cypher::engine::fs

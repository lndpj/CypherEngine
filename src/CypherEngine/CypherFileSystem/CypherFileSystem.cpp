/*======================================================================
   File: CypherFileSystem.cpp
   Project: CypherEngine
   Author: ksiric <email@example.com>
   Created: 2026-04-26 15:53:16
   Last Modified by: ksiric
   Last Modified: 2026-06-12 14:10:53
   ---------------------------------------------------------------------
   Description:

   ---------------------------------------------------------------------
   License:
   Company:
   Version: 0.1.0
 ======================================================================
																	   */

#include "CypherEngine/CypherFileSystem/CypherFileSystem.h"
#include "CypherEngine/CypherFileSystem/CypherFileSystem_Runtime.h"
#include "CypherEngine/CypherLog/CypherLog.h"
#include "CypherEngine/CypherPak/CypherPak.h"

#include <algorithm>
#include <cstdio>           // stdio file handles and read/write operations.
#include <cstring>          // strcmp / strncpy for fixed path buffers.
#include <filesystem>       // Path probing and directory creation.
#include <limits>
#include <new>
#include <system_error>     // std::error_code for non-throwing filesystem calls.

namespace cypher::engine::fs {

namespace {

struct package_file_state_t {
	common::u8 *data{ nullptr };
	common::u64 size{ 0u };
};

fs_error_t PakErrorToFs( const pak::pak_error_t error )
{
	switch ( error ) {
	case pak::pak_error_t::OK:
		return fs_error_t::OK;
	case pak::pak_error_t::ERR_INVALID_ARGUMENT:
		return fs_error_t::ERR_INVALID_ARGUMENT;
	case pak::pak_error_t::ERR_INVALID_PATH:
		return fs_error_t::ERR_INVALID_PATH;
	case pak::pak_error_t::ERR_INVALID_HANDLE:
		return fs_error_t::ERR_INVALID_HANDLE;
	case pak::pak_error_t::ERR_ENTRY_NOT_FOUND:
	case pak::pak_error_t::ERR_PATH_NOT_FOUND:
		return fs_error_t::ERR_PATH_NOT_FOUND;
	case pak::pak_error_t::ERR_BUFFER_TOO_SMALL:
		return fs_error_t::ERR_BUFFER_TOO_SMALL;
	case pak::pak_error_t::ERR_OUT_OF_MEMORY:
		return fs_error_t::ERR_OUT_OF_MEMORY;
	case pak::pak_error_t::ERR_PERMISSION_DENIED:
		return fs_error_t::ERR_PERMISSION_DENIED;
	case pak::pak_error_t::ERR_UNSUPPORTED_COMPRESSION:
		return fs_error_t::ERR_UNSUPPORTED_BACKEND;
	default:
		return fs_error_t::ERR_IO_ERROR;
	}
}

void ClearRuntimeState( runtime_state_t &state )
{
	std::memset( &state, 0, sizeof( state ) );
}

}       // namespace

/*
================
CypherFileSystem_Init
================
*/
fs_error_t CypherFileSystem_Init() {
	std::lock_guard<std::recursive_mutex> lock( CypherFileSystem_RuntimeMutex() );
	runtime_state_t &state = CypherFileSystem_RuntimeState();
	if ( state.initialized ) {
		LOG_WARNING( log::channel_t::FS, "filesystem init requested while already initialized." );
		return fs_error_t::ERR_IS_INIT;
	}

	ClearRuntimeState( state );
	state.initialized = true;
	state.next_mount_handle = 1u;
	state.next_watch_handle = 1u;

	LOG_INFO( log::channel_t::FS, "filesystem initialized." );

	return fs_error_t::OK;
}

/*
================
CypherFileSystem_Shutdown
================
*/
fs_error_t CypherFileSystem_Shutdown() {
	std::lock_guard<std::recursive_mutex> lock( CypherFileSystem_RuntimeMutex() );
	runtime_state_t &state = CypherFileSystem_RuntimeState();
	if ( !state.initialized ) {
		LOG_WARNING( log::channel_t::FS, "filesystem shutdown requested while not initialized." );
		return fs_error_t::ERR_NOT_INIT;
	}
	LOG_INFO( log::channel_t::FS, "filesystem shutdown: mounts=%u.", state.mount_count );
	while ( state.mount_count > 0u ) {
		const mount_handle_t mount = state.mounts[0].handle;
		( void )CypherFileSystem_Unmount( mount );
	}
	ClearRuntimeState( state );
	return fs_error_t::OK;
}

/*
================
CypherFileSystem_IsInitialized
================
*/
bool CypherFileSystem_IsInitialized() {
	std::lock_guard<std::recursive_mutex> lock( CypherFileSystem_RuntimeMutex() );
	return CypherFileSystem_RuntimeState().initialized;
}

/*
================
CypherFileSystem_SetWritePath
================
*/
fs_error_t CypherFileSystem_SetWritePath( const char *physical_path ) {
	std::lock_guard<std::recursive_mutex> lock( CypherFileSystem_RuntimeMutex() );
	runtime_state_t &state = CypherFileSystem_RuntimeState();
	if ( !state.initialized ) {
		LOG_ERROR( log::channel_t::FS, "set write path failed: filesystem is not initialized." );
		return fs_error_t::ERR_NOT_INIT;
	}
	if ( physical_path == nullptr || physical_path[0] == '\0' ) {
		LOG_ERROR( log::channel_t::FS, "set write path failed: invalid physical path." );
		return fs_error_t::ERR_INVALID_PATH;
	}
	const common::u32 physical_path_length = static_cast<common::u32>( std::strlen( physical_path ) );
	if ( physical_path_length + 1u > sizeof( state.write_path ) ) {
		LOG_ERROR( log::channel_t::FS, "set write path failed for '%s': path is too long.", physical_path );
		return fs_error_t::ERR_BUFFER_TOO_SMALL;
	}

	std::error_code ec{};
	std::filesystem::create_directories( physical_path, ec );
	if ( ec ) {
		LOG_ERROR( log::channel_t::FS, "set write path failed for '%s': create directories failed.", physical_path );
		return fs_error_t::ERR_IO_ERROR;
	}
	if ( !std::filesystem::is_directory( physical_path, ec ) || ec ) {
		LOG_ERROR( log::channel_t::FS, "set write path failed for '%s': path is not a directory.", physical_path );
		return ec ? fs_error_t::ERR_IO_ERROR : fs_error_t::ERR_NOT_DIRECTORY;
	}

	std::memcpy( state.write_path, physical_path, physical_path_length + 1u );
	LOG_INFO( log::channel_t::FS, "write path set to '%s'.", state.write_path );
	return fs_error_t::OK;
}

/*
================
CypherFileSystem_GetWritePath
================
*/
const char *CypherFileSystem_GetWritePath() {
	std::lock_guard<std::recursive_mutex> lock( CypherFileSystem_RuntimeMutex() );
	const runtime_state_t &state = CypherFileSystem_RuntimeState();
	if ( !state.initialized ) {
		return nullptr;
	}
	if ( state.write_path[0] == '\0' ) {
		return nullptr;
	}
	return state.write_path;
}

/*
================
CypherFileSystem_OpenModeToCMode
================
*/
static const char *CypherFileSystem_OpenModeToCMode( const open_mode_t mode ) {
	switch ( mode ) {
	case open_mode_t::READ_TEXT:
		return "rb";
	case open_mode_t::READ_BINARY:
		return "rb";
	case open_mode_t::WRITE_TEXT:
		return "wb";
	case open_mode_t::WRITE_BINARY:
		return "wb";
	case open_mode_t::APPEND_TEXT:
		return "ab";
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
fs_error_t CypherFileSystem_Open( const char *virtual_path, open_mode_t mode, file_t &file ) {
	std::lock_guard<std::recursive_mutex> lock( CypherFileSystem_RuntimeMutex() );
	if ( !CypherFileSystem_RuntimeState().initialized ) {
		return fs_error_t::ERR_NOT_INIT;
	}
	file = {};
	if ( virtual_path == nullptr || virtual_path[0] == '\0' ) {
		return fs_error_t::ERR_INVALID_PATH;
	}
	const char *c_mode = CypherFileSystem_OpenModeToCMode( mode );
	if ( c_mode == nullptr ) {
		return fs_error_t::ERR_INVALID_MODE;
	}
	const bool read_mode = mode == open_mode_t::READ_TEXT || mode == open_mode_t::READ_BINARY;
	const bool write_mode = mode == open_mode_t::WRITE_TEXT || mode == open_mode_t::WRITE_BINARY;
	const bool append_mode = mode == open_mode_t::APPEND_TEXT || mode == open_mode_t::APPEND_BINARY;
	char resolved_path[CYPHER_FILESYSTEM_MAX_PATH_LENGTH]{};
	if ( read_mode ) {
		resolved_file_t resolved_file{};
		const fs_error_t err = CypherFileSystem_ResolveReadableFile( virtual_path, resolved_file );
		if ( err != fs_error_t::OK ) {
			return err;
		}
		if ( resolved_file.backend == file_backend_t::PACKAGE_FILE ) {
			package_file_state_t *package_state = new ( std::nothrow ) package_file_state_t();
			if ( package_state == nullptr ) {
				return fs_error_t::ERR_OUT_OF_MEMORY;
			}

			package_state->size = resolved_file.file_size;
			if ( package_state->size > 0u ) {
				if ( package_state->size > static_cast<common::u64>( std::numeric_limits<common::usize>::max() ) ) {
					delete package_state;
					return fs_error_t::ERR_OUT_OF_MEMORY;
				}
				package_state->data = new ( std::nothrow ) common::u8[static_cast<common::usize>( package_state->size )];
				if ( package_state->data == nullptr ) {
					delete package_state;
					return fs_error_t::ERR_OUT_OF_MEMORY;
				}
			}

			common::u64 bytes_read = 0u;
			const pak::pak_error_t read_result = pak::CypherPak_ReadFileByIndex(
				*static_cast<pak::pak_reader_t *>( resolved_file.package_reader ),
				resolved_file.package_file_index,
				package_state->data,
				package_state->size,
				bytes_read );
			if ( read_result != pak::pak_error_t::OK || bytes_read != package_state->size ) {
				delete[] package_state->data;
				delete package_state;
				return read_result != pak::pak_error_t::OK ? PakErrorToFs( read_result ) : fs_error_t::ERR_FILE_READ_FAILED;
			}

			file.backend = file_backend_t::PACKAGE_FILE;
			file.native_handle = package_state;
			file.readable = true;
			file.writable = false;
			file.cursor = 0u;
			file.size = package_state->size;
			CypherFileSystem_RuntimeState().stats.open_count++;
			return fs_error_t::OK;
		}
		if ( resolved_file.backend != file_backend_t::OS_FILE ) {
			return fs_error_t::ERR_UNSUPPORTED_BACKEND;
		}
		std::memcpy( resolved_path, resolved_file.physical_path, std::strlen( resolved_file.physical_path ) + 1u );
	} else if ( write_mode || append_mode ) {
		const fs_error_t build_path_result = CypherFileSystem_BuildWritePath( virtual_path, resolved_path, sizeof( resolved_path ) );
		if ( build_path_result != fs_error_t::OK ) {
			return build_path_result;
		}
		std::error_code ec{};
		const std::filesystem::path parent_path = std::filesystem::path( resolved_path ).parent_path();
		if ( !parent_path.empty() ) {
			std::filesystem::create_directories( parent_path, ec );
			if ( ec ) {
				return fs_error_t::ERR_IO_ERROR;
			}
		}
	}
	std::FILE *native_file = std::fopen( resolved_path, c_mode );
	if ( native_file == nullptr ) {
		return fs_error_t::ERR_FILE_OPEN_FAILED;
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
			return fs_error_t::ERR_IO_ERROR;
		}
	}
	if ( append_mode ) {
		file.cursor = file.size;
	}

	CypherFileSystem_RuntimeState().stats.open_count++;
	return fs_error_t::OK;
}

/*
================
CypherFileSystem_Close
================
*/
fs_error_t CypherFileSystem_Close( file_t &file ) {
	std::lock_guard<std::recursive_mutex> lock( CypherFileSystem_RuntimeMutex() );
	if ( !CypherFileSystem_RuntimeState().initialized ) {
		return fs_error_t::ERR_NOT_INIT;
	}

	if ( file.native_handle == nullptr ) {
		return fs_error_t::ERR_INVALID_HANDLE;
	}

	if ( file.backend == file_backend_t::PACKAGE_FILE ) {
		package_file_state_t *package_state = static_cast<package_file_state_t *>( file.native_handle );
		delete[] package_state->data;
		delete package_state;
		file = {};
		CypherFileSystem_RuntimeState().stats.close_count++;
		return fs_error_t::OK;
	}

	if ( file.backend != file_backend_t::OS_FILE ) {
		return fs_error_t::ERR_UNSUPPORTED_BACKEND;
	}

	std::FILE *native_file = static_cast<std::FILE *>( file.native_handle );
	if ( std::fclose( native_file ) != 0 ) {
		file = {};
		return fs_error_t::ERR_FILE_CLOSE_FAILED;
	}

	file = {};
	CypherFileSystem_RuntimeState().stats.close_count++;

	return fs_error_t::OK;
}

/*
================
CypherFileSystem_Read
================
*/
fs_error_t CypherFileSystem_Read( file_t &file, void *buffer, common::u64 bytes_to_read, common::u64 &bytes_read_out ) {
	std::lock_guard<std::recursive_mutex> lock( CypherFileSystem_RuntimeMutex() );
	bytes_read_out = 0u;

	if ( !CypherFileSystem_RuntimeState().initialized ) {
		return fs_error_t::ERR_NOT_INIT;
	}

	if ( file.native_handle == nullptr ) {
		return fs_error_t::ERR_INVALID_HANDLE;
	}

	if ( !file.readable ) {
		return fs_error_t::ERR_PERMISSION_DENIED;
	}

	if ( bytes_to_read == 0u ) {
		return fs_error_t::OK;
	}

	if ( buffer == nullptr ) {
		return fs_error_t::ERR_INVALID_ARGUMENT;
	}

	if ( file.backend == file_backend_t::PACKAGE_FILE ) {
		package_file_state_t *package_state = static_cast<package_file_state_t *>( file.native_handle );
		const common::u64 remaining = file.cursor < package_state->size ? package_state->size - file.cursor : 0u;
		const common::u64 bytes_to_copy = std::min( bytes_to_read, remaining );
		if ( bytes_to_copy > 0u ) {
			std::memcpy( buffer, package_state->data + file.cursor, static_cast<common::usize>( bytes_to_copy ) );
		}
		bytes_read_out = bytes_to_copy;
		file.cursor += bytes_to_copy;
		CypherFileSystem_RuntimeState().stats.read_count++;
		CypherFileSystem_RuntimeState().stats.bytes_read += bytes_read_out;
		return fs_error_t::OK;
	}

	if ( file.backend != file_backend_t::OS_FILE ) {
		return fs_error_t::ERR_UNSUPPORTED_BACKEND;
	}

	const std::size_t read_size = static_cast<std::size_t>( bytes_to_read );

	if ( static_cast<common::u64>( read_size ) != bytes_to_read ) {
		return fs_error_t::ERR_INVALID_ARGUMENT;
	}

	std::FILE *native_file = static_cast<std::FILE *>( file.native_handle );
	const std::size_t bytes_read = std::fread( buffer, 1u, read_size, native_file );

	bytes_read_out = static_cast<common::u64>( bytes_read );
	file.cursor += bytes_read_out;
	CypherFileSystem_RuntimeState().stats.read_count++;
	CypherFileSystem_RuntimeState().stats.bytes_read += bytes_read_out;

	if ( bytes_read != read_size && std::ferror( native_file ) != 0 ) {
		return fs_error_t::ERR_FILE_READ_FAILED;
	}

	return fs_error_t::OK;
}

/*
================
CypherFileSystem_Write
================
*/
fs_error_t CypherFileSystem_Write( file_t &file, const void *buffer, common::u64 bytes_to_write, common::u64 &bytes_written_out ) {
	std::lock_guard<std::recursive_mutex> lock( CypherFileSystem_RuntimeMutex() );
	bytes_written_out = 0u;

	if ( !CypherFileSystem_RuntimeState().initialized ) {
		return fs_error_t::ERR_NOT_INIT;
	}
	if ( file.backend == file_backend_t::PACKAGE_FILE ) {
		return fs_error_t::ERR_PERMISSION_DENIED;
	}
	if ( file.backend != file_backend_t::OS_FILE ) {
		return fs_error_t::ERR_UNSUPPORTED_BACKEND;
	}
	if ( file.native_handle == nullptr ) {
		return fs_error_t::ERR_INVALID_HANDLE;
	}
	if ( !file.writable ) {
		return fs_error_t::ERR_PERMISSION_DENIED;
	}
	if ( bytes_to_write == 0u ) {
		return fs_error_t::OK;
	}
	if ( buffer == nullptr ) {
		return fs_error_t::ERR_INVALID_ARGUMENT;
	}
	const std::size_t write_size = static_cast<std::size_t>( bytes_to_write );

	if ( static_cast<common::u64>( write_size ) != bytes_to_write ) {
		return fs_error_t::ERR_INVALID_ARGUMENT;
	}
	std::FILE *native_file = static_cast<std::FILE *>( file.native_handle );
	const std::size_t bytes_written = std::fwrite( buffer, 1u, write_size, native_file );

	bytes_written_out = static_cast<common::u64>( bytes_written );
	file.cursor += bytes_written_out;
	CypherFileSystem_RuntimeState().stats.write_count++;
	CypherFileSystem_RuntimeState().stats.bytes_written += bytes_written_out;

	if ( file.cursor > file.size ) {
		file.size = file.cursor;
	}

	if ( bytes_written != write_size ) {
		return fs_error_t::ERR_FILE_WRITE_FAILED;
	}

	return fs_error_t::OK;
}

/*
================
CypherFileSystem_Seek
================
*/
fs_error_t CypherFileSystem_Seek( file_t &file, common::i64 offset, seek_origin_t origin ) {
	std::lock_guard<std::recursive_mutex> lock( CypherFileSystem_RuntimeMutex() );
	if ( !CypherFileSystem_RuntimeState().initialized ) {
		return fs_error_t::ERR_NOT_INIT;
	}

	if ( file.native_handle == nullptr ) {
		return fs_error_t::ERR_INVALID_HANDLE;
	}

	if ( file.backend == file_backend_t::PACKAGE_FILE ) {
		common::i64 base = 0;
		switch ( origin ) {
		case seek_origin_t::CYPHER_FILESYSTEM_SEEK_START:
			base = 0;
			break;
		case seek_origin_t::CYPHER_FILESYSTEM_SEEK_CURRENT:
			base = static_cast<common::i64>( file.cursor );
			break;
		case seek_origin_t::CYPHER_FILESYSTEM_SEEK_END:
			base = static_cast<common::i64>( file.size );
			break;
		default:
			return fs_error_t::ERR_INVALID_ARGUMENT;
		}

		const common::i64 target = base + offset;
		if ( target < 0 || static_cast<common::u64>( target ) > file.size ) {
			return fs_error_t::ERR_FILE_SEEK_FAILED;
		}

		file.cursor = static_cast<common::u64>( target );
		return fs_error_t::OK;
	}

	if ( file.backend != file_backend_t::OS_FILE ) {
		return fs_error_t::ERR_UNSUPPORTED_BACKEND;
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
		return fs_error_t::ERR_INVALID_ARGUMENT;
	}

	std::FILE *native_file = static_cast<std::FILE *>( file.native_handle );

	if ( std::fseek( native_file, static_cast<long>( offset ), c_origin ) != 0 ) {
		return fs_error_t::ERR_FILE_SEEK_FAILED;
	}

	const long position = std::ftell( native_file );

	if ( position < 0 ) {
		return fs_error_t::ERR_FILE_TELL_FAILED;
	}

	file.cursor = static_cast<common::u64>( position );

	return fs_error_t::OK;
}

/*
================
CypherFileSystem_Tell
================
*/
fs_error_t CypherFileSystem_Tell( file_t &file, common::u64 &out_position ) {
	std::lock_guard<std::recursive_mutex> lock( CypherFileSystem_RuntimeMutex() );
	out_position = 0u;

	if ( !CypherFileSystem_RuntimeState().initialized ) {
		return fs_error_t::ERR_NOT_INIT;
	}

	if ( file.native_handle == nullptr ) {
		return fs_error_t::ERR_INVALID_HANDLE;
	}

	if ( file.backend == file_backend_t::PACKAGE_FILE ) {
		out_position = file.cursor;
		return fs_error_t::OK;
	}

	if ( file.backend != file_backend_t::OS_FILE ) {
		return fs_error_t::ERR_UNSUPPORTED_BACKEND;
	}

	std::FILE *native_file = static_cast<std::FILE *>( file.native_handle );
	const long position = std::ftell( native_file );

	if ( position < 0 ) {
		return fs_error_t::ERR_FILE_TELL_FAILED;
	}

	out_position = static_cast<common::u64>( position );
	file.cursor = out_position;

	return fs_error_t::OK;
}

/*
================
CypherFileSystem_Flush
================
*/
fs_error_t CypherFileSystem_Flush( file_t &file ) {
	std::lock_guard<std::recursive_mutex> lock( CypherFileSystem_RuntimeMutex() );
	if ( !CypherFileSystem_RuntimeState().initialized ) {
		return fs_error_t::ERR_NOT_INIT;
	}
	if ( file.backend == file_backend_t::PACKAGE_FILE ) {
		return fs_error_t::ERR_PERMISSION_DENIED;
	}
	if ( file.backend != file_backend_t::OS_FILE ) {
		return fs_error_t::ERR_UNSUPPORTED_BACKEND;
	}
	if ( file.native_handle == nullptr ) {
		return fs_error_t::ERR_INVALID_HANDLE;
	}
	if ( !file.writable ) {
		return fs_error_t::ERR_PERMISSION_DENIED;
	}

	std::FILE *native_file = static_cast<std::FILE *>( file.native_handle );
	return std::fflush( native_file ) == 0 ? fs_error_t::OK : fs_error_t::ERR_FILE_WRITE_FAILED;
}

/*
================
CypherFileSystem_ReadEntireFile
================
*/
fs_error_t CypherFileSystem_ReadEntireFile( const char *virtual_path, void *buffer, common::u64 bytes_to_read, common::u64 &bytes_read_out ) {
	std::lock_guard<std::recursive_mutex> lock( CypherFileSystem_RuntimeMutex() );
	bytes_read_out = 0u;

	if ( !CypherFileSystem_RuntimeState().initialized ) {
		LOG_ERROR( log::channel_t::FS, "read entire file failed for '%s': filesystem is not initialized.", virtual_path ? virtual_path : "<null>" );
		return fs_error_t::ERR_NOT_INIT;
	}

	if ( buffer == nullptr ) {
		LOG_ERROR( log::channel_t::FS, "read entire file failed for '%s': output buffer is null.", virtual_path ? virtual_path : "<null>" );
		return fs_error_t::ERR_INVALID_ARGUMENT;
	}

	file_t file{};
	fs_error_t err = CypherFileSystem_Open( virtual_path, open_mode_t::READ_BINARY, file );

	if ( err != fs_error_t::OK ) {
		LOG_ERROR( log::channel_t::FS, "read entire file failed for '%s': open failed: %s.", virtual_path ? virtual_path : "<null>", CypherFileSystem_ErrorDesc( err ) );
		return err;
	}

	if ( file.size > bytes_to_read ) {
		CypherFileSystem_Close( file );
		LOG_ERROR( log::channel_t::FS, "read entire file failed for '%s': buffer too small, file=%llu bytes, buffer=%llu bytes.",
		                  virtual_path ? virtual_path : "<null>",
		                  static_cast<unsigned long long>( file.size ),
		                  static_cast<unsigned long long>( bytes_to_read ) );
		return fs_error_t::ERR_BUFFER_TOO_SMALL;
	}

	if ( file.size == 0u ) {
		return CypherFileSystem_Close( file );
	}

	const common::u64 expected_size = file.size;
	err = CypherFileSystem_Read( file, buffer, expected_size, bytes_read_out );
	const fs_error_t close_err = CypherFileSystem_Close( file );

	if ( err != fs_error_t::OK ) {
		LOG_ERROR( log::channel_t::FS, "read entire file failed for '%s': read failed: %s.", virtual_path ? virtual_path : "<null>", CypherFileSystem_ErrorDesc( err ) );
		return err;
	}

	if ( close_err != fs_error_t::OK ) {
		LOG_ERROR( log::channel_t::FS, "read entire file failed for '%s': close failed: %s.", virtual_path ? virtual_path : "<null>", CypherFileSystem_ErrorDesc( close_err ) );
		return close_err;
	}

	if ( bytes_read_out != expected_size ) {
		LOG_ERROR( log::channel_t::FS, "read entire file failed for '%s': short read, expected=%llu, actual=%llu.",
		                  virtual_path ? virtual_path : "<null>",
		                  static_cast<unsigned long long>( expected_size ),
		                  static_cast<unsigned long long>( bytes_read_out ) );
		return fs_error_t::ERR_FILE_READ_FAILED;
	}

	return fs_error_t::OK;
}

/*
================
CypherFileSystem_WriteEntireFile
================
*/
fs_error_t CypherFileSystem_WriteEntireFile( const char *virtual_path, const void *buffer, common::u64 bytes_to_write ) {
	std::lock_guard<std::recursive_mutex> lock( CypherFileSystem_RuntimeMutex() );
	if ( bytes_to_write != 0u && buffer == nullptr ) {
		return fs_error_t::ERR_INVALID_ARGUMENT;
	}

	file_t file{};
	fs_error_t err = CypherFileSystem_Open( virtual_path, open_mode_t::WRITE_BINARY, file );
	if ( err != fs_error_t::OK ) {
		return err;
	}

	common::u64 bytes_written = 0u;
	err = CypherFileSystem_Write( file, buffer, bytes_to_write, bytes_written );
	const fs_error_t flush_err = err == fs_error_t::OK ? CypherFileSystem_Flush( file ) : fs_error_t::OK;
	const fs_error_t close_err = CypherFileSystem_Close( file );

	if ( err != fs_error_t::OK ) {
		return err;
	}
	if ( flush_err != fs_error_t::OK ) {
		return flush_err;
	}
	if ( close_err != fs_error_t::OK ) {
		return close_err;
	}
	return bytes_written == bytes_to_write ? fs_error_t::OK : fs_error_t::ERR_FILE_WRITE_FAILED;
}

/*
================
CypherFileSystem_AppendEntireFile
================
*/
fs_error_t CypherFileSystem_AppendEntireFile( const char *virtual_path, const void *buffer, common::u64 bytes_to_write ) {
	std::lock_guard<std::recursive_mutex> lock( CypherFileSystem_RuntimeMutex() );
	if ( bytes_to_write != 0u && buffer == nullptr ) {
		return fs_error_t::ERR_INVALID_ARGUMENT;
	}

	file_t file{};
	fs_error_t err = CypherFileSystem_Open( virtual_path, open_mode_t::APPEND_BINARY, file );
	if ( err != fs_error_t::OK ) {
		return err;
	}

	common::u64 bytes_written = 0u;
	err = CypherFileSystem_Write( file, buffer, bytes_to_write, bytes_written );
	const fs_error_t flush_err = err == fs_error_t::OK ? CypherFileSystem_Flush( file ) : fs_error_t::OK;
	const fs_error_t close_err = CypherFileSystem_Close( file );

	if ( err != fs_error_t::OK ) {
		return err;
	}
	if ( flush_err != fs_error_t::OK ) {
		return flush_err;
	}
	if ( close_err != fs_error_t::OK ) {
		return close_err;
	}
	return bytes_written == bytes_to_write ? fs_error_t::OK : fs_error_t::ERR_FILE_WRITE_FAILED;
}

} // namespace cypher::engine::fs

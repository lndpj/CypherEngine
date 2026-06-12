/*======================================================================
   File: CypherFileSystem_Directory.cpp
   Project: CYPHER
   Author: ksiric <email@example.com>
   Created: 2026-06-12 13:18:53
   Last Modified by: ksiric
   Last Modified: 2026-06-12 13:35:14
   ---------------------------------------------------------------------
   Description:

   ---------------------------------------------------------------------
   License:
   Company:
   Version: 0.1.0
 ======================================================================
                                                                       */

#include "CypherEngine/CypherFileSystem/CypherFileSystem_Runtime.h"

#include <filesystem>
#include <limits>
#include <system_error>
#include <vector>

namespace cypher::engine::fs
{

/*
================
CypherFileSystem_CreateDirectory

Creates a directory below the configured write path. Existing directories are OK.
================
*/
fs_error_t CypherFileSystem_CreateDirectory( const char *virtual_path )
{
    std::lock_guard<std::recursive_mutex> lock( CypherFileSystem_RuntimeMutex() );
    char physical_path[CYPHER_FILESYSTEM_MAX_PATH_LENGTH]{};
    const fs_error_t build_result = CypherFileSystem_BuildWritePath( virtual_path, physical_path, sizeof( physical_path ) );
    if ( build_result != fs_error_t::OK ) {
        return build_result;
    }

    std::error_code ec{};
    if ( std::filesystem::exists( physical_path, ec ) ) {
        if ( ec ) {
            return fs_error_t::ERR_IO_ERROR;
        }
        return std::filesystem::is_directory( physical_path, ec ) && !ec ? fs_error_t::OK : fs_error_t::ERR_NOT_DIRECTORY;
    }
    if ( ec ) {
        return fs_error_t::ERR_IO_ERROR;
    }

    std::filesystem::create_directories( physical_path, ec );
    if ( ec ) {
        return fs_error_t::ERR_IO_ERROR;
    }

    return fs_error_t::OK;
}

/*
================
CypherFileSystem_DeleteFile

Deletes a file below the configured write path. It never deletes mounted read-only content.
================
*/
fs_error_t CypherFileSystem_DeleteFile( const char *virtual_path )
{
    std::lock_guard<std::recursive_mutex> lock( CypherFileSystem_RuntimeMutex() );
    char physical_path[CYPHER_FILESYSTEM_MAX_PATH_LENGTH]{};
    const fs_error_t build_result = CypherFileSystem_BuildWritePath( virtual_path, physical_path, sizeof( physical_path ) );
    if ( build_result != fs_error_t::OK ) {
        return build_result;
    }

    std::error_code ec{};
    if ( !std::filesystem::exists( physical_path, ec ) ) {
        return ec ? fs_error_t::ERR_IO_ERROR : fs_error_t::ERR_PATH_NOT_FOUND;
    }
    if ( std::filesystem::is_directory( physical_path, ec ) ) {
        return ec ? fs_error_t::ERR_IO_ERROR : fs_error_t::ERR_NOT_FILE;
    }

    const bool removed = std::filesystem::remove( physical_path, ec );
    if ( ec ) {
        return fs_error_t::ERR_IO_ERROR;
    }

    return removed ? fs_error_t::OK : fs_error_t::ERR_PATH_NOT_FOUND;
}

/*
================
CypherFileSystem_RemoveDirectory

Removes an empty directory below the configured write path.
================
*/
fs_error_t CypherFileSystem_RemoveDirectory( const char *virtual_path )
{
    std::lock_guard<std::recursive_mutex> lock( CypherFileSystem_RuntimeMutex() );
    char physical_path[CYPHER_FILESYSTEM_MAX_PATH_LENGTH]{};
    const fs_error_t build_result = CypherFileSystem_BuildWritePath( virtual_path, physical_path, sizeof( physical_path ) );
    if ( build_result != fs_error_t::OK ) {
        return build_result;
    }

    std::error_code ec{};
    if ( !std::filesystem::exists( physical_path, ec ) ) {
        return ec ? fs_error_t::ERR_IO_ERROR : fs_error_t::ERR_PATH_NOT_FOUND;
    }
    if ( !std::filesystem::is_directory( physical_path, ec ) || ec ) {
        return ec ? fs_error_t::ERR_IO_ERROR : fs_error_t::ERR_NOT_DIRECTORY;
    }

    const bool removed = std::filesystem::remove( physical_path, ec );
    if ( ec == std::errc::directory_not_empty ) {
        return fs_error_t::ERR_DIRECTORY_NOT_EMPTY;
    }
    if ( ec ) {
        return fs_error_t::ERR_IO_ERROR;
    }

    return removed ? fs_error_t::OK : fs_error_t::ERR_DIRECTORY_NOT_EMPTY;
}

/*
================
CypherFileSystem_RemoveDirectoryTree

Recursively removes a directory below the configured write path.
================
*/
fs_error_t CypherFileSystem_RemoveDirectoryTree( const char *virtual_path )
{
    std::lock_guard<std::recursive_mutex> lock( CypherFileSystem_RuntimeMutex() );
    char physical_path[CYPHER_FILESYSTEM_MAX_PATH_LENGTH]{};
    const fs_error_t build_result = CypherFileSystem_BuildWritePath( virtual_path, physical_path, sizeof( physical_path ) );
    if ( build_result != fs_error_t::OK ) {
        return build_result;
    }

    std::error_code ec{};
    if ( !std::filesystem::exists( physical_path, ec ) ) {
        return ec ? fs_error_t::ERR_IO_ERROR : fs_error_t::ERR_PATH_NOT_FOUND;
    }
    if ( !std::filesystem::is_directory( physical_path, ec ) || ec ) {
        return ec ? fs_error_t::ERR_IO_ERROR : fs_error_t::ERR_NOT_DIRECTORY;
    }

    std::filesystem::remove_all( physical_path, ec );
    return ec ? fs_error_t::ERR_IO_ERROR : fs_error_t::OK;
}

/*
================
CypherFileSystem_Rename

Renames a write-path file or directory. Destination must not already exist.
================
*/
fs_error_t CypherFileSystem_Rename( const char *from_virtual_path, const char *to_virtual_path )
{
    std::lock_guard<std::recursive_mutex> lock( CypherFileSystem_RuntimeMutex() );
    char from_physical_path[CYPHER_FILESYSTEM_MAX_PATH_LENGTH]{};
    char to_physical_path[CYPHER_FILESYSTEM_MAX_PATH_LENGTH]{};

    fs_error_t build_result = CypherFileSystem_BuildWritePath( from_virtual_path, from_physical_path, sizeof( from_physical_path ) );
    if ( build_result != fs_error_t::OK ) {
        return build_result;
    }

    build_result = CypherFileSystem_BuildWritePath( to_virtual_path, to_physical_path, sizeof( to_physical_path ) );
    if ( build_result != fs_error_t::OK ) {
        return build_result;
    }

    std::error_code ec{};
    if ( !std::filesystem::exists( from_physical_path, ec ) ) {
        return ec ? fs_error_t::ERR_IO_ERROR : fs_error_t::ERR_PATH_NOT_FOUND;
    }
    if ( std::filesystem::exists( to_physical_path, ec ) ) {
        return ec ? fs_error_t::ERR_IO_ERROR : fs_error_t::ERR_ALREADY_EXISTS;
    }

    const std::filesystem::path parent_path = std::filesystem::path( to_physical_path ).parent_path();
    if ( !parent_path.empty() ) {
        std::filesystem::create_directories( parent_path, ec );
        if ( ec ) {
            return fs_error_t::ERR_IO_ERROR;
        }
    }

    std::filesystem::rename( from_physical_path, to_physical_path, ec );
    return ec ? fs_error_t::ERR_IO_ERROR : fs_error_t::OK;
}

/*
================
CypherFileSystem_CopyFile

Copies from the readable virtual view into the configured write path.
================
*/
fs_error_t CypherFileSystem_CopyFile( const char *from_virtual_path, const char *to_virtual_path )
{
    std::lock_guard<std::recursive_mutex> lock( CypherFileSystem_RuntimeMutex() );
    char to_physical_path[CYPHER_FILESYSTEM_MAX_PATH_LENGTH]{};

    file_info_t source_info{};
    fs_error_t result = CypherFileSystem_GetFileInfo( from_virtual_path, source_info );
    if ( result != fs_error_t::OK ) {
        return result;
    }
    if ( source_info.is_directory ) {
        return fs_error_t::ERR_NOT_FILE;
    }

    result = CypherFileSystem_BuildWritePath( to_virtual_path, to_physical_path, sizeof( to_physical_path ) );
    if ( result != fs_error_t::OK ) {
        return result;
    }

    std::error_code ec{};
    if ( std::filesystem::exists( to_physical_path, ec ) ) {
        return ec ? fs_error_t::ERR_IO_ERROR : fs_error_t::ERR_ALREADY_EXISTS;
    }

    const std::filesystem::path parent_path = std::filesystem::path( to_physical_path ).parent_path();
    if ( !parent_path.empty() ) {
        std::filesystem::create_directories( parent_path, ec );
        if ( ec ) {
            return fs_error_t::ERR_IO_ERROR;
        }
    }

    if ( source_info.backend == file_backend_t::OS_FILE && source_info.resolved_path[0] != '\0' ) {
        const bool copied = std::filesystem::copy_file( source_info.resolved_path, to_physical_path, std::filesystem::copy_options::none, ec );
        if ( ec ) {
            return fs_error_t::ERR_IO_ERROR;
        }
        return copied ? fs_error_t::OK : fs_error_t::ERR_FILE_WRITE_FAILED;
    }

    if ( source_info.file_size == 0u ) {
        return CypherFileSystem_WriteEntireFile( to_virtual_path, nullptr, 0u );
    }

    if ( source_info.file_size > static_cast<common::u64>( std::numeric_limits<common::usize>::max() ) ) {
        return fs_error_t::ERR_OUT_OF_MEMORY;
    }

    std::vector<common::u8> buffer( static_cast<common::usize>( source_info.file_size ) );
    common::u64 bytes_read = 0u;
    result = CypherFileSystem_ReadEntireFile( from_virtual_path, buffer.data(), source_info.file_size, bytes_read );
    if ( result != fs_error_t::OK ) {
        return result;
    }
    if ( bytes_read != source_info.file_size ) {
        return fs_error_t::ERR_FILE_READ_FAILED;
    }

    return CypherFileSystem_WriteEntireFile( to_virtual_path, buffer.data(), bytes_read );
}

/*
================
CypherFileSystem_DirectoryExists

Checks the readable mounted filesystem view.
================
*/
bool CypherFileSystem_DirectoryExists( const char *virtual_path )
{
    std::lock_guard<std::recursive_mutex> lock( CypherFileSystem_RuntimeMutex() );
    if ( !CypherFileSystem_RuntimeState().initialized ) {
        return false;
    }

    common::u32 entry_count = 0u;
    const fs_error_t result = CypherFileSystem_ListDirectory( virtual_path, nullptr, 0u, entry_count );
    return result == fs_error_t::OK || result == fs_error_t::ERR_BUFFER_TOO_SMALL;
}

}       // namespace cypher::engine::fs

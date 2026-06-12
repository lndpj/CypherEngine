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
#include <system_error>

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
CypherFileSystem_Rename

Renames a write-path file or directory. Destination must not already exist.
================
*/
fs_error_t CypherFileSystem_Rename( const char *from_virtual_path, const char *to_virtual_path )
{
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
CypherFileSystem_DirectoryExists

Checks the readable mounted filesystem view.
================
*/
bool CypherFileSystem_DirectoryExists( const char *virtual_path )
{
    if ( !CypherFileSystem_RuntimeState().initialized ) {
        return false;
    }

    char resolved_path[CYPHER_FILESYSTEM_MAX_PATH_LENGTH]{};
    if ( CypherFileSystem_ResolvePath( virtual_path, resolved_path, sizeof( resolved_path ) ) != fs_error_t::OK ) {
        return false;
    }

    std::error_code ec{};
    return std::filesystem::is_directory( resolved_path, ec ) && !ec;
}

}       // namespace cypher::engine::fs

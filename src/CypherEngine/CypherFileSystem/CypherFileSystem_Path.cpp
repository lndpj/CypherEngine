/*======================================================================
   File: CypherFileSystem_Path.cpp
   Project: CYPHER
   Author: ksiric <email@example.com>
   Created: 2026-06-12 10:24:36
   Last Modified by: ksiric
   Last Modified: 2026-06-12 15:16:02
   ---------------------------------------------------------------------
   Description:
       
   ---------------------------------------------------------------------
   License: 
   Company: 
   Version: 0.1.0
 ======================================================================
                                                                       */

#include "CypherEngine/CypherFileSystem/CypherFileSystem.h"

#include <cctype>           // for isalpha and other isFuncs
#include <cstdio>           // for IO
#include <cstring>          // for string manipulations

/*
 * @Note: Internal helpers usually put them in anonymous namespaces as they are file-local.
 *        So we treat them that way as well, this goes for all internal helper functions.
 *        You can see that I do not put the Prefixed namings for them and that is how I can
 *        distinguish them from being helpers or core funcs.
 */
namespace {

bool IsInvalidVirtualPathChar( char c )
{
    unsigned char u = static_cast<unsigned char>( c );
    if ( u < 32 ) {
        return true;
    }
    if ( u == 127 ) {
        return true;
    }
    // @note: I am rejecting unicode characters for now as well.
    if ( u >= 128 ) {
        return true;
    }
    if ( c == ':' ) {
        return true;
    }
    if ( c == '*' ) {
        return true;
    }
    if ( c == '?' ) {
        return true;
    }
    if ( c == '"' ) {
        return true;
    }
    if ( c == '<' ) {
        return true;
    }
    if ( c == '>' ) {
        return true;
    }
    if ( c == '|' ) {
        return true;
    }
    if ( c == ' ' ) {
        return true;
    }
    return false;
}

}       // namespace

namespace cypher::engine::fs
{

/*
================
CypherFileSystem_NormalizeVirtualPath
================
*/
fs_error_t CypherFileSystem_NormalizeVirtualPath( const char *virtual_path, char *out_path, common::u32 out_path_size )
{
    if ( virtual_path == nullptr || virtual_path[0] == '\0' ) {
        return fs_error_t::ERR_INVALID_PATH;
    }
    if ( out_path == nullptr || out_path_size == 0u ) {
        return fs_error_t::ERR_INVALID_ARGUMENT;
    }
    out_path[0] = '\0';

    if ( virtual_path[0] == '/' || virtual_path[0] == '\\' ) {
        return fs_error_t::ERR_INVALID_PATH;
    }

    if ( std::isalpha(static_cast<unsigned char>( virtual_path[0] ) ) && virtual_path[1] == ':' ) {
        return fs_error_t::ERR_INVALID_PATH;
    }
    const char *cursor = virtual_path;

    common::u32 write_index = 0u;

    while ( *cursor != '\0' ) {
        // @Skipping the characters for slashes
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
            return fs_error_t::ERR_INVALID_PATH;
        }
        if ( write_index != 0u ) {
            if ( write_index + 1u >= out_path_size ) {
                out_path[0] = '\0';
                return fs_error_t::ERR_BUFFER_TOO_SMALL;
            }

            out_path[write_index] = '/';
            ++write_index;
        }
        // @copying - we are copying the segment characters into this out buffer
        for ( common::u32 i = 0u; i < segment_length; ++i ) {
            char c = segment_start[i];
            if ( IsInvalidVirtualPathChar( c ) ) {
                out_path[0] = '\0';
                return fs_error_t::ERR_INVALID_PATH;
            }
            if ( c >= 'A' && c <= 'Z' ) {
                c = static_cast<char>( c - 'A' + 'a' );
            }
            if ( write_index + 1u >= out_path_size ) {
                out_path[0] = '\0';
                return fs_error_t::ERR_BUFFER_TOO_SMALL;
            }
            out_path[write_index] = c;
            ++write_index;
        }

        out_path[write_index] = '\0';
    }

    if ( write_index == 0u ) {
        return fs_error_t::ERR_INVALID_PATH;
    }

    return fs_error_t::OK;
}

/*
================
CypherFileSystem_NormalizeVirtualRoot
================
*/
fs_error_t CypherFileSystem_NormalizeVirtualRoot( const char *virtual_root, char *out_root, common::u32 out_size )
{
    if ( out_root == nullptr || out_size == 0u ) {
        return fs_error_t::ERR_INVALID_ARGUMENT;
    }
    
    out_root[0] = '\0';

    if ( virtual_root == nullptr ) {
        return fs_error_t::ERR_INVALID_ROOT;
    }
    
    if ( virtual_root[0] == '\0' ) {
        return fs_error_t::OK;
    }
    
    return CypherFileSystem_NormalizeVirtualPath( virtual_root, out_root, out_size );
}

/*
================
CypherFileSystem_VirtualPathStartsWithRoot
================
*/
bool CypherFileSystem_VirtualPathStartsWithRoot( const char *virtual_path, const char *virtual_root, const char **out_relative_path )
{
    if ( out_relative_path != nullptr ) {
        *out_relative_path = nullptr;
    }
    if ( virtual_path == nullptr || virtual_root == nullptr ) {
        return false;
    }
    if ( virtual_root[0] == '\0' ) {
        if ( out_relative_path != nullptr ) {
            *out_relative_path = virtual_path;
        }
        return true;
    }
    
    common::usize root_len = std::strlen( virtual_root );
    
    if ( std::strncmp( virtual_path, virtual_root, root_len ) != 0 ) {
        return false;
    }
    char next_char = virtual_path[root_len];
    if ( next_char != '\0' && next_char != '/' ) {
        return false;
    }
    const char *relative_path = virtual_path + root_len;
    
    if ( relative_path[0] == '/' ) {
        ++relative_path;
    } 
    
    if ( out_relative_path != nullptr ) {
        *out_relative_path = relative_path;
    }
    
    return true;
}

/*
================
CypherFileSystem_BuildPhysicalPath
================
*/
fs_error_t CypherFileSystem_BuildPhysicalPath( const char *physical_root, const char *relative_path, char *out_path, common::u32 out_path_size )
{
    if ( out_path == nullptr || out_path_size == 0u ) {
        return fs_error_t::ERR_INVALID_ARGUMENT;
    }    
    out_path[0] = '\0';
    if ( physical_root == nullptr || physical_root[0] == '\0' ) {
        return fs_error_t::ERR_INVALID_ROOT;
    }
    if ( relative_path == nullptr ) {
        return fs_error_t::ERR_INVALID_PATH;
    }
    
    if ( relative_path[0] == '\0' ) {
        common::usize required = std::strlen( physical_root ) + 1;
        if ( required > out_path_size ) {
            return fs_error_t::ERR_BUFFER_TOO_SMALL;
        }
        std::memcpy( out_path, physical_root, required );
        return fs_error_t::OK;
    } 
    
    const int written = std::snprintf( out_path, out_path_size, "%s/%s", physical_root, relative_path );
    
    if ( written < 0 ) {
        out_path[0] = '\0';
        // @Note: here we can also have ERR_INVALID_PATH as well but lets keep IO for now.
        return fs_error_t::ERR_IO_ERROR;
    }
    if ( static_cast<common::u32>( written ) >= out_path_size ) {
        out_path[0] = '\0';
        return fs_error_t::ERR_BUFFER_TOO_SMALL;
    }
    
    return fs_error_t::OK;
}

bool CypherFileSystem_IsValidVirtualPath( const char *virtual_path )
{
    char normalized_path[CYPHER_FILESYSTEM_MAX_PATH_LENGTH]{};
    fs_error_t result = CypherFileSystem_NormalizeVirtualPath( virtual_path, normalized_path, sizeof( normalized_path ) );
    return ( result == fs_error_t::OK );
}

fs_error_t CypherFileSystem_PathJoin(
    const char *left,
    const char *right,
    char *out_path,
    common::u32 out_path_size )
{
    if ( out_path == nullptr || out_path_size == 0u ) {
        return fs_error_t::ERR_INVALID_ARGUMENT;
    }
    out_path[0] = '\0';
    if ( left == nullptr || left[0] == '\0' ) {
        return fs_error_t::ERR_INVALID_PATH;
    }
    if ( right == nullptr || right[0] == '\0' ) {
        return fs_error_t::ERR_INVALID_PATH;
    }
    char normalized_left[CYPHER_FILESYSTEM_MAX_PATH_LENGTH]{};
    char normalized_right[CYPHER_FILESYSTEM_MAX_PATH_LENGTH]{};

    fs_error_t err = CypherFileSystem_NormalizeVirtualPath( left, normalized_left, sizeof( normalized_left ) );
    if ( err != fs_error_t::OK ) {
        return err;
    }
    err = CypherFileSystem_NormalizeVirtualPath( right, normalized_right, sizeof( normalized_right ) );
    if ( err != fs_error_t::OK ) {
        return err;
    }
    const common::usize left_len = std::strlen( normalized_left );
    const common::usize right_len = std::strlen( normalized_right );

    const common::usize required = left_len + 1u + right_len + 1u;
    if ( required > out_path_size ) {
        return fs_error_t::ERR_BUFFER_TOO_SMALL;
    }
    // @safetyFirst! safe now we can safely copy things
    std::memcpy( out_path, normalized_left, left_len );
    out_path[left_len] = '/';
    std::memcpy( out_path + left_len + 1, normalized_right, right_len + 1 );

    return fs_error_t::OK;
}

}       // namespace cypher::engine::fs

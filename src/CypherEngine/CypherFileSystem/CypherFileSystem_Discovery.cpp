#include "CypherEngine/CypherFileSystem/CypherFileSystem_Runtime.h"
#include "CypherEngine/CypherPak/CypherPak.h"

#include <algorithm>
#include <cstring>
#include <filesystem>
#include <string>
#include <system_error>
#include <vector>

namespace cypher::engine::fs
{

namespace {

bool CopyString( char *out, const common::u32 out_size, const char *text )
{
    if ( out == nullptr || out_size == 0u || text == nullptr ) {
        return false;
    }

    const common::usize text_len = std::strlen( text );
    if ( text_len + 1u > out_size ) {
        out[0] = '\0';
        return false;
    }

    std::memcpy( out, text, text_len + 1u );
    return true;
}

bool IsHiddenEntryName( const char *name )
{
    return name != nullptr && name[0] == '.';
}

bool EntryAlreadyAdded( const directory_entry_t *entries, const common::u32 entry_count, const char *virtual_path )
{
    if ( entries == nullptr || virtual_path == nullptr ) {
        return false;
    }

    for ( common::u32 i = 0u; i < entry_count; ++i ) {
        if ( std::strcmp( entries[i].virtual_path, virtual_path ) == 0 ) {
            return true;
        }
    }

    return false;
}

fs_error_t BuildVirtualChildPath(
    const char *virtual_dir,
    const char *child_name,
    char *out_path,
    const common::u32 out_path_size )
{
    if ( virtual_dir == nullptr || virtual_dir[0] == '\0' ) {
        return CopyString( out_path, out_path_size, child_name ) ? fs_error_t::OK : fs_error_t::ERR_BUFFER_TOO_SMALL;
    }

    return CypherFileSystem_PathJoin( virtual_dir, child_name, out_path, out_path_size );
}

fs_error_t FillEntryFromPhysicalPath(
    const char *virtual_path,
    const char *physical_path,
    directory_entry_t &out_entry )
{
    out_entry = {};

    if ( !CopyString( out_entry.virtual_path, sizeof( out_entry.virtual_path ), virtual_path ) ) {
        return fs_error_t::ERR_BUFFER_TOO_SMALL;
    }

    const char *basename = CypherFileSystem_PathBasename( virtual_path );
    if ( basename == nullptr || !CopyString( out_entry.name, sizeof( out_entry.name ), basename ) ) {
        return fs_error_t::ERR_BUFFER_TOO_SMALL;
    }

    std::error_code ec{};
    const bool is_directory = std::filesystem::is_directory( physical_path, ec );
    if ( ec ) {
        return fs_error_t::ERR_IO_ERROR;
    }

    out_entry.type = is_directory ? directory_entry_type_t::DIRECTORY : directory_entry_type_t::FILE;
    if ( !is_directory ) {
        out_entry.size = static_cast<common::u64>( std::filesystem::file_size( physical_path, ec ) );
        if ( ec ) {
            out_entry.size = 0u;
            return fs_error_t::ERR_IO_ERROR;
        }
    }

    return fs_error_t::OK;
}

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
    default:
        return fs_error_t::ERR_IO_ERROR;
    }
}

fs_error_t AddDirectoryEntry(
    const directory_entry_t &entry,
    directory_entry_t *entries,
    const common::u32 max_entries,
    common::u32 &out_entry_count,
    bool &out_overflow )
{
    if ( EntryAlreadyAdded( entries, out_entry_count < max_entries ? out_entry_count : max_entries, entry.virtual_path ) ) {
        return fs_error_t::OK;
    }

    if ( out_entry_count < max_entries && entries != nullptr ) {
        entries[out_entry_count] = entry;
    } else {
        out_overflow = true;
    }

    ++out_entry_count;
    return fs_error_t::OK;
}

fs_error_t AddMountedChildRoot(
    const char *requested_root,
    const char *mounted_root,
    directory_entry_t *entries,
    const common::u32 max_entries,
    common::u32 &out_entry_count,
    bool &out_overflow )
{
    const char *relative_path = nullptr;
    if ( requested_root[0] != '\0' && !CypherFileSystem_VirtualPathStartsWithRoot( mounted_root, requested_root, &relative_path ) ) {
        return fs_error_t::OK;
    }

    if ( requested_root[0] == '\0' ) {
        relative_path = mounted_root;
    }
    if ( relative_path == nullptr || relative_path[0] == '\0' ) {
        return fs_error_t::OK;
    }

    char child_name[CYPHER_FILESYSTEM_MAX_PATH_LENGTH]{};
    common::u32 write_index = 0u;
    const char *cursor = relative_path;
    while ( *cursor != '\0' && *cursor != '/' ) {
        if ( write_index + 1u >= sizeof( child_name ) ) {
            return fs_error_t::ERR_BUFFER_TOO_SMALL;
        }
        child_name[write_index++] = *cursor++;
    }
    child_name[write_index] = '\0';

    char child_virtual_path[CYPHER_FILESYSTEM_MAX_PATH_LENGTH]{};
    fs_error_t result = BuildVirtualChildPath( requested_root, child_name, child_virtual_path, sizeof( child_virtual_path ) );
    if ( result != fs_error_t::OK ) {
        return result;
    }

    directory_entry_t entry{};
    if ( !CopyString( entry.name, sizeof( entry.name ), child_name ) ||
         !CopyString( entry.virtual_path, sizeof( entry.virtual_path ), child_virtual_path ) ) {
        return fs_error_t::ERR_BUFFER_TOO_SMALL;
    }

    entry.type = directory_entry_type_t::DIRECTORY;
    return AddDirectoryEntry( entry, entries, max_entries, out_entry_count, out_overflow );
}

fs_error_t AddPackagePathChild(
    const char *requested_root,
    const char *relative_dir,
    const pak::pak_file_info_t &file_info,
    directory_entry_t *entries,
    const common::u32 max_entries,
    common::u32 &out_entry_count,
    bool &out_overflow,
    bool &out_found_source )
{
    const char *tail = file_info.virtual_path;

    if ( relative_dir != nullptr && relative_dir[0] != '\0' ) {
        const common::usize relative_len = std::strlen( relative_dir );
        if ( std::strncmp( file_info.virtual_path, relative_dir, relative_len ) != 0 ||
             file_info.virtual_path[relative_len] != '/' ) {
            return fs_error_t::OK;
        }
        tail = file_info.virtual_path + relative_len + 1u;
    }

    if ( tail == nullptr || tail[0] == '\0' ) {
        return fs_error_t::OK;
    }

    out_found_source = true;

    char child_name[CYPHER_FILESYSTEM_MAX_PATH_LENGTH]{};
    common::u32 write_index = 0u;
    const char *cursor = tail;
    while ( *cursor != '\0' && *cursor != '/' ) {
        if ( write_index + 1u >= sizeof( child_name ) ) {
            return fs_error_t::ERR_BUFFER_TOO_SMALL;
        }
        child_name[write_index++] = *cursor++;
    }
    child_name[write_index] = '\0';

    const bool child_is_directory = *cursor == '/';

    char child_virtual_path[CYPHER_FILESYSTEM_MAX_PATH_LENGTH]{};
    fs_error_t result = BuildVirtualChildPath( requested_root, child_name, child_virtual_path, sizeof( child_virtual_path ) );
    if ( result != fs_error_t::OK ) {
        return result;
    }

    directory_entry_t entry{};
    if ( !CopyString( entry.name, sizeof( entry.name ), child_name ) ||
         !CopyString( entry.virtual_path, sizeof( entry.virtual_path ), child_virtual_path ) ) {
        return fs_error_t::ERR_BUFFER_TOO_SMALL;
    }
    entry.type = child_is_directory ? directory_entry_type_t::DIRECTORY : directory_entry_type_t::FILE;
    entry.size = child_is_directory ? 0u : file_info.unpacked_size;
    entry.modified_time = static_cast<std::time_t>( file_info.modified_time_utc );

    return AddDirectoryEntry( entry, entries, max_entries, out_entry_count, out_overflow );
}

bool MatchWildcard( const char *text, const char *pattern )
{
    if ( text == nullptr || pattern == nullptr ) {
        return false;
    }

    const char *star = nullptr;
    const char *text_after_star = nullptr;

    while ( *text != '\0' ) {
        if ( *pattern == '?' || *pattern == *text ) {
            ++text;
            ++pattern;
            continue;
        }

        if ( *pattern == '*' ) {
            star = pattern++;
            text_after_star = text;
            continue;
        }

        if ( star != nullptr ) {
            pattern = star + 1;
            text = ++text_after_star;
            continue;
        }

        return false;
    }

    while ( *pattern == '*' ) {
        ++pattern;
    }

    return *pattern == '\0';
}

bool IsInvalidFindPatternChar( const char c )
{
    const unsigned char u = static_cast<unsigned char>( c );
    if ( u < 32u || u == 127u || u >= 128u ) {
        return true;
    }

    switch ( c ) {
    case ':':
    case '"':
    case '<':
    case '>':
    case '|':
    case ' ':
        return true;
    default:
        return false;
    }
}

fs_error_t NormalizeFindPattern( const char *pattern, char *out_pattern, const common::u32 out_pattern_size )
{
    if ( pattern == nullptr || pattern[0] == '\0' ) {
        return fs_error_t::ERR_INVALID_ARGUMENT;
    }
    if ( out_pattern == nullptr || out_pattern_size == 0u ) {
        return fs_error_t::ERR_INVALID_ARGUMENT;
    }

    out_pattern[0] = '\0';

    common::u32 write_index = 0u;
    const char *cursor = pattern;
    while ( *cursor != '\0' ) {
        char c = *cursor;
        if ( c == '/' || c == '\\' ) {
            return fs_error_t::ERR_INVALID_ARGUMENT;
        }
        if ( c != '*' && c != '?' && IsInvalidFindPatternChar( c ) ) {
            return fs_error_t::ERR_INVALID_ARGUMENT;
        }
        if ( c >= 'A' && c <= 'Z' ) {
            c = static_cast<char>( c - 'A' + 'a' );
        }
        if ( write_index + 1u >= out_pattern_size ) {
            out_pattern[0] = '\0';
            return fs_error_t::ERR_BUFFER_TOO_SMALL;
        }
        out_pattern[write_index++] = c;
        ++cursor;
    }

    out_pattern[write_index] = '\0';
    return fs_error_t::OK;
}

fs_error_t FindFilesRecursive(
    const char *virtual_root,
    const char *pattern,
    const common::u32 flags,
    directory_entry_t *entries,
    const common::u32 max_entries,
    common::u32 &out_entry_count,
    bool &out_overflow )
{
    common::u32 child_count = 0u;
    fs_error_t result = CypherFileSystem_ListDirectory( virtual_root, nullptr, 0u, child_count );
    if ( result == fs_error_t::ERR_PATH_NOT_FOUND ) {
        return fs_error_t::OK;
    }
    if ( result != fs_error_t::OK && result != fs_error_t::ERR_BUFFER_TOO_SMALL ) {
        return result;
    }
    if ( child_count == 0u ) {
        return fs_error_t::OK;
    }

    std::vector<directory_entry_t> children( child_count );
    while ( true ) {
        common::u32 listed_child_count = 0u;
        result = CypherFileSystem_ListDirectory(
            virtual_root,
            children.data(),
            static_cast<common::u32>( children.size() ),
            listed_child_count );

        if ( result == fs_error_t::ERR_BUFFER_TOO_SMALL ) {
            children.resize( listed_child_count );
            continue;
        }
        if ( result != fs_error_t::OK ) {
            return result;
        }

        children.resize( listed_child_count );
        break;
    }

    const common::u32 match_flags = ( flags & ( CYPHER_FILESYSTEM_FIND_FILES | CYPHER_FILESYSTEM_FIND_DIRECTORIES ) ) != 0u
        ? flags
        : flags | CYPHER_FILESYSTEM_FIND_FILES | CYPHER_FILESYSTEM_FIND_DIRECTORIES;

    for ( common::u32 i = 0u; i < static_cast<common::u32>( children.size() ); ++i ) {
        const directory_entry_t &child = children[i];
        const bool is_hidden = IsHiddenEntryName( child.name );
        if ( is_hidden && ( flags & CYPHER_FILESYSTEM_FIND_INCLUDE_HIDDEN ) == 0u ) {
            continue;
        }

        const bool type_matches =
            ( child.type == directory_entry_type_t::FILE && ( match_flags & CYPHER_FILESYSTEM_FIND_FILES ) != 0u ) ||
            ( child.type == directory_entry_type_t::DIRECTORY && ( match_flags & CYPHER_FILESYSTEM_FIND_DIRECTORIES ) != 0u );

        if ( type_matches && MatchWildcard( child.name, pattern ) ) {
            result = AddDirectoryEntry( child, entries, max_entries, out_entry_count, out_overflow );
            if ( result != fs_error_t::OK ) {
                return result;
            }
        }

        if ( child.type == directory_entry_type_t::DIRECTORY && ( flags & CYPHER_FILESYSTEM_FIND_RECURSIVE ) != 0u ) {
            result = FindFilesRecursive( child.virtual_path, pattern, flags, entries, max_entries, out_entry_count, out_overflow );
            if ( result != fs_error_t::OK ) {
                return result;
            }
        }
    }

    return result == fs_error_t::ERR_BUFFER_TOO_SMALL ? fs_error_t::OK : result;
}

}       // namespace

fs_error_t CypherFileSystem_ListDirectory(
    const char *virtual_path,
    directory_entry_t *entries,
    common::u32 max_entries,
    common::u32 &out_entry_count )
{
    runtime_state_t &state = CypherFileSystem_RuntimeState();
    std::lock_guard<std::recursive_mutex> lock( CypherFileSystem_RuntimeMutex() );
    out_entry_count = 0u;

    if ( !state.initialized ) {
        return fs_error_t::ERR_NOT_INIT;
    }
    if ( entries == nullptr && max_entries != 0u ) {
        return fs_error_t::ERR_INVALID_ARGUMENT;
    }

    char normalized_root[CYPHER_FILESYSTEM_MAX_PATH_LENGTH]{};
    const fs_error_t root_result = CypherFileSystem_NormalizeVirtualRoot( virtual_path, normalized_root, sizeof( normalized_root ) );
    if ( root_result != fs_error_t::OK ) {
        return root_result;
    }

    bool found_source = false;
    bool overflow = false;

    for ( common::u32 i = 0u; i < state.mount_count; ++i ) {
        const mount_t &mount = state.mounts[i];

        const char *relative_path = nullptr;
        if ( mount.type == mount_type_t::CYPHER_FILESYSTEM_DIRECTORY &&
             CypherFileSystem_VirtualPathStartsWithRoot( normalized_root, mount.virtual_root, &relative_path ) ) {
            char physical_path[CYPHER_FILESYSTEM_MAX_PATH_LENGTH]{};
            fs_error_t result = CypherFileSystem_BuildPhysicalPath( mount.physical_root, relative_path, physical_path, sizeof( physical_path ) );
            if ( result != fs_error_t::OK ) {
                return result;
            }

            std::error_code ec{};
            if ( !std::filesystem::exists( physical_path, ec ) ) {
                if ( ec ) {
                    return fs_error_t::ERR_IO_ERROR;
                }
                continue;
            }
            if ( !std::filesystem::is_directory( physical_path, ec ) || ec ) {
                return ec ? fs_error_t::ERR_IO_ERROR : fs_error_t::ERR_NOT_DIRECTORY;
            }

            found_source = true;
            for ( const std::filesystem::directory_entry &physical_entry : std::filesystem::directory_iterator( physical_path, ec ) ) {
                if ( ec ) {
                    return fs_error_t::ERR_IO_ERROR;
                }

                const std::string raw_name = physical_entry.path().filename().string();
                char normalized_name[CYPHER_FILESYSTEM_MAX_PATH_LENGTH]{};
                if ( CypherFileSystem_NormalizeVirtualPath( raw_name.c_str(), normalized_name, sizeof( normalized_name ) ) != fs_error_t::OK ) {
                    continue;
                }

                char child_virtual_path[CYPHER_FILESYSTEM_MAX_PATH_LENGTH]{};
                result = BuildVirtualChildPath( normalized_root, normalized_name, child_virtual_path, sizeof( child_virtual_path ) );
                if ( result != fs_error_t::OK ) {
                    return result;
                }

                directory_entry_t entry{};
                result = FillEntryFromPhysicalPath( child_virtual_path, physical_entry.path().string().c_str(), entry );
                if ( result != fs_error_t::OK ) {
                    return result;
                }

                result = AddDirectoryEntry( entry, entries, max_entries, out_entry_count, overflow );
                if ( result != fs_error_t::OK ) {
                    return result;
                }
            }
        } else if ( mount.type == mount_type_t::CYPHER_FILESYSTEM_PACKAGE &&
                    CypherFileSystem_VirtualPathStartsWithRoot( normalized_root, mount.virtual_root, &relative_path ) ) {
            pak::pak_reader_t *reader = static_cast<pak::pak_reader_t *>( mount.package_reader );
            if ( reader == nullptr ) {
                continue;
            }

            common::u32 package_file_count = 0u;
            pak::pak_error_t count_result = pak::CypherPak_GetFileCount( *reader, package_file_count );
            if ( count_result != pak::pak_error_t::OK ) {
                return PakErrorToFs( count_result );
            }

            for ( common::u32 package_index = 0u; package_index < package_file_count; ++package_index ) {
                pak::pak_file_info_t file_info{};
                const pak::pak_error_t info_result = pak::CypherPak_GetFileInfo( *reader, package_index, file_info );
                if ( info_result != pak::pak_error_t::OK ) {
                    return PakErrorToFs( info_result );
                }

                fs_error_t result = AddPackagePathChild(
                    normalized_root,
                    relative_path,
                    file_info,
                    entries,
                    max_entries,
                    out_entry_count,
                    overflow,
                    found_source );
                if ( result != fs_error_t::OK ) {
                    return result;
                }
            }
        } else if ( CypherFileSystem_VirtualPathStartsWithRoot( mount.virtual_root, normalized_root, nullptr ) ) {
            found_source = true;
            const fs_error_t result = AddMountedChildRoot( normalized_root, mount.virtual_root, entries, max_entries, out_entry_count, overflow );
            if ( result != fs_error_t::OK ) {
                return result;
            }
        }
    }

    if ( !found_source ) {
        state.stats.failed_lookup_count++;
        return fs_error_t::ERR_PATH_NOT_FOUND;
    }

    return overflow ? fs_error_t::ERR_BUFFER_TOO_SMALL : fs_error_t::OK;
}

fs_error_t CypherFileSystem_FindFiles(
    const char *virtual_root,
    const char *pattern,
    common::u32 flags,
    directory_entry_t *entries,
    common::u32 max_entries,
    common::u32 &out_entry_count )
{
    std::lock_guard<std::recursive_mutex> lock( CypherFileSystem_RuntimeMutex() );
    out_entry_count = 0u;

    if ( !CypherFileSystem_RuntimeState().initialized ) {
        return fs_error_t::ERR_NOT_INIT;
    }
    if ( entries == nullptr && max_entries != 0u ) {
        return fs_error_t::ERR_INVALID_ARGUMENT;
    }

    char normalized_pattern[CYPHER_FILESYSTEM_MAX_PATTERN_LENGTH]{};
    fs_error_t result = NormalizeFindPattern( pattern, normalized_pattern, sizeof( normalized_pattern ) );
    if ( result != fs_error_t::OK ) {
        return result;
    }

    bool overflow = false;
    result = FindFilesRecursive( virtual_root, normalized_pattern, flags, entries, max_entries, out_entry_count, overflow );
    if ( result != fs_error_t::OK ) {
        return result;
    }

    const common::u32 sort_count = out_entry_count < max_entries ? out_entry_count : max_entries;
    if ( entries != nullptr && ( flags & CYPHER_FILESYSTEM_FIND_SORT_BY_NAME ) != 0u ) {
        std::sort( entries, entries + sort_count, []( const directory_entry_t &a, const directory_entry_t &b ) {
            return std::strcmp( a.virtual_path, b.virtual_path ) < 0;
        } );
    }

    return overflow ? fs_error_t::ERR_BUFFER_TOO_SMALL : fs_error_t::OK;
}

}       // namespace cypher::engine::fs

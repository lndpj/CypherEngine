#include "CypherEngine/CypherFileSystem/CypherFileSystem.h"
#include "CypherEngine/CypherPak/CypherPak.h"

#include <cstdio>
#include <cstring>
#include <filesystem>
#include <string>

namespace fs = cypher::engine::fs;
namespace pak = cypher::engine::pak;

namespace {

bool Check( bool condition, const char *message )
{
    if ( condition ) {
        return true;
    }

    std::fprintf( stderr, "filesystem smoke failed: %s\n", message );
    return false;
}

bool CheckError( fs::fs_error_t actual, fs::fs_error_t expected, const char *message )
{
    if ( actual == expected ) {
        return true;
    }

    std::fprintf(
        stderr,
        "filesystem smoke failed: %s: expected %s, got %s\n",
        message,
        fs::CypherFileSystem_ErrorName( expected ),
        fs::CypherFileSystem_ErrorName( actual ) );
    return false;
}

bool WritePhysicalTextFile( const std::filesystem::path &path, const char *text )
{
    std::error_code ec{};
    std::filesystem::create_directories( path.parent_path(), ec );
    if ( ec ) {
        return false;
    }

    std::FILE *file = std::fopen( path.string().c_str(), "wb" );
    if ( file == nullptr ) {
        return false;
    }

    const std::size_t text_len = std::strlen( text );
    const bool write_ok = std::fwrite( text, 1u, text_len, file ) == text_len;
    const bool close_ok = std::fclose( file ) == 0;
    return write_ok && close_ok;
}

bool HasDirectoryEntry(
    const fs::directory_entry_t *entries,
    const cypher::engine::common::u32 entry_count,
    const char *virtual_path )
{
    for ( cypher::engine::common::u32 i = 0u; i < entry_count; ++i ) {
        if ( std::strcmp( entries[i].virtual_path, virtual_path ) == 0 ) {
            return true;
        }
    }

    return false;
}

bool TraceResolvedThroughMount( const fs::resolve_trace_t &trace, const fs::mount_handle_t mount )
{
    if ( !trace.resolved ) {
        return false;
    }

    for ( cypher::engine::common::u32 i = 0u; i < trace.checked_mount_count; ++i ) {
        const fs::resolve_trace_entry_t &entry = trace.entries[i];
        if ( entry.mount == mount && entry.root_matched && entry.path_exists ) {
            return true;
        }
    }

    return false;
}

}       // namespace

int main()
{
    const std::filesystem::path root_path =
        std::filesystem::temp_directory_path() / "CypherFileSystem_Smoke";

    std::error_code ec{};
    std::filesystem::remove_all( root_path, ec );
    std::filesystem::create_directories( root_path, ec );

    if ( ec ) {
        std::fprintf( stderr, "filesystem smoke failed: temp directory setup failed\n" );
        return 1;
    }

    const std::string write_path = root_path.string();

    if ( !CheckError( fs::CypherFileSystem_Init(), fs::fs_error_t::OK, "init" ) ) {
        return 1;
    }
    if ( !CheckError( fs::CypherFileSystem_SetWritePath( write_path.c_str() ), fs::fs_error_t::OK, "set write path" ) ) {
        return 1;
    }

    char path_buffer[fs::CYPHER_FILESYSTEM_MAX_PATH_LENGTH]{};
    if ( !CheckError( fs::CypherFileSystem_NormalizeVirtualPath( "Profiles\\Player1\\CONFIG.CFG", path_buffer, sizeof( path_buffer ) ), fs::fs_error_t::OK, "normalize virtual path" ) ) {
        return 1;
    }
    if ( !Check( std::strcmp( path_buffer, "profiles/player1/config.cfg" ) == 0, "normalized path value" ) ) {
        return 1;
    }
    if ( !Check( fs::CypherFileSystem_IsValidVirtualPath( "textures/walls/stone.dds" ), "valid virtual path" ) ) {
        return 1;
    }
    struct invalid_virtual_path_case_t {
        const char *path;
        const char *message;
    };
    const invalid_virtual_path_case_t invalid_virtual_paths[] = {
        { "/absolute/path.cfg", "invalid absolute virtual path" },
        { "C:/absolute/path.cfg", "invalid drive-letter virtual path" },
        { "profiles/../secret.txt", "invalid parent traversal virtual path" }
    };
    for ( const invalid_virtual_path_case_t &invalid_path : invalid_virtual_paths ) {
        if ( !Check( !fs::CypherFileSystem_IsValidVirtualPath( invalid_path.path ), invalid_path.message ) ) {
            return 1;
        }
        if ( !CheckError( fs::CypherFileSystem_NormalizeVirtualPath( invalid_path.path, path_buffer, sizeof( path_buffer ) ), fs::fs_error_t::ERR_INVALID_PATH, invalid_path.message ) ) {
            return 1;
        }
    }
    if ( !CheckError( fs::CypherFileSystem_PathJoin( "profiles", "Player1\\CONFIG.CFG", path_buffer, sizeof( path_buffer ) ), fs::fs_error_t::OK, "path join" ) ) {
        return 1;
    }
    if ( !Check( std::strcmp( path_buffer, "profiles/player1/config.cfg" ) == 0, "path join value" ) ) {
        return 1;
    }
    if ( !Check( std::strcmp( fs::CypherFileSystem_PathBasename( "profiles/player1/config.cfg" ), "config.cfg" ) == 0, "path basename" ) ) {
        return 1;
    }
    if ( !CheckError( fs::CypherFileSystem_PathDirname( "Profiles\\Player1\\CONFIG.CFG", path_buffer, sizeof( path_buffer ) ), fs::fs_error_t::OK, "path dirname" ) ) {
        return 1;
    }
    if ( !Check( std::strcmp( path_buffer, "profiles/player1" ) == 0, "path dirname value" ) ) {
        return 1;
    }
    if ( !Check( std::strcmp( fs::CypherFileSystem_PathExtension( "profiles/player1/config.cfg" ), ".cfg" ) == 0, "path extension" ) ) {
        return 1;
    }
    if ( !CheckError( fs::CypherFileSystem_PathWithoutExtension( "Profiles\\Player1\\CONFIG.CFG", path_buffer, sizeof( path_buffer ) ), fs::fs_error_t::OK, "path without extension" ) ) {
        return 1;
    }
    if ( !Check( std::strcmp( path_buffer, "profiles/player1/config" ) == 0, "path without extension value" ) ) {
        return 1;
    }
    if ( !Check( fs::CypherFileSystem_PathHasExtension( "profiles/player1/config.cfg", "CFG" ), "path has extension" ) ) {
        return 1;
    }

    if ( !CheckError( fs::CypherFileSystem_CreateDirectory( "profiles/player1" ), fs::fs_error_t::OK, "create directory" ) ) {
        return 1;
    }

    fs::file_t file{};
    if ( !CheckError( fs::CypherFileSystem_Open( "profiles/player1/config.cfg", fs::open_mode_t::WRITE_TEXT, file ), fs::fs_error_t::OK, "open write" ) ) {
        return 1;
    }

    const char text[] = "name player1\n";
    cypher::engine::common::u64 bytes_written = 0u;
    if ( !CheckError( fs::CypherFileSystem_Write( file, text, std::strlen( text ), bytes_written ), fs::fs_error_t::OK, "write file" ) ) {
        return 1;
    }
    if ( !Check( bytes_written == std::strlen( text ), "write count" ) ) {
        return 1;
    }
    if ( !CheckError( fs::CypherFileSystem_Close( file ), fs::fs_error_t::OK, "close file" ) ) {
        return 1;
    }

    fs::mount_handle_t read_mount = fs::CYPHER_FILESYSTEM_INVALID_MOUNT;
    fs::mount_handle_t writable_mount = 123u;
    if ( !CheckError( fs::CypherFileSystem_MountDirectoryWithHandle( "writable", write_path.c_str(), fs::CYPHER_FILESYSTEM_MOUNT_WRITABLE, 0u, writable_mount ), fs::fs_error_t::ERR_NOT_IMPLEMENTED, "writable directory mount not implemented" ) ) {
        return 1;
    }
    if ( !Check( writable_mount == fs::CYPHER_FILESYSTEM_INVALID_MOUNT, "writable directory mount leaves handle invalid" ) ) {
        return 1;
    }
    if ( !CheckError( fs::CypherFileSystem_MountDirectory( "mixed", write_path.c_str(), fs::CYPHER_FILESYSTEM_MOUNT_READ_ONLY | fs::CYPHER_FILESYSTEM_MOUNT_WRITABLE, 0u ), fs::fs_error_t::ERR_NOT_IMPLEMENTED, "mixed writable directory mount not implemented" ) ) {
        return 1;
    }
    if ( !CheckError( fs::CypherFileSystem_MountDirectoryWithHandle( "", write_path.c_str(), fs::CYPHER_FILESYSTEM_MOUNT_READ_ONLY, 0u, read_mount ), fs::fs_error_t::OK, "mount write path for read view" ) ) {
        return 1;
    }
    if ( !Check( read_mount != fs::CYPHER_FILESYSTEM_INVALID_MOUNT, "mount handle assigned" ) ) {
        return 1;
    }
    fs::mount_info_t mount_info{};
    if ( !CheckError( fs::CypherFileSystem_GetMountInfoByHandle( read_mount, mount_info ), fs::fs_error_t::OK, "get mount info by handle" ) ) {
        return 1;
    }
    if ( !Check( mount_info.handle == read_mount && mount_info.priority == 0u, "mount info values" ) ) {
        return 1;
    }

    const std::filesystem::path base_content_path = root_path / "BaseContent";
    const std::filesystem::path mod_content_path = root_path / "ModContent";
    if ( !Check( WritePhysicalTextFile( base_content_path / "shared.cfg", "base\n" ), "write base overlay file" ) ) {
        return 1;
    }
    if ( !Check( WritePhysicalTextFile( base_content_path / "base_only.cfg", "base\n" ), "write base-only overlay file" ) ) {
        return 1;
    }
    if ( !Check( WritePhysicalTextFile( mod_content_path / "shared.cfg", "mod\n" ), "write mod overlay file" ) ) {
        return 1;
    }
    if ( !Check( WritePhysicalTextFile( mod_content_path / "mod_only.cfg", "mod\n" ), "write mod-only overlay file" ) ) {
        return 1;
    }

    fs::mount_handle_t base_mount = fs::CYPHER_FILESYSTEM_INVALID_MOUNT;
    if ( !CheckError( fs::CypherFileSystem_MountDirectoryWithHandle( "game", base_content_path.string().c_str(), fs::CYPHER_FILESYSTEM_MOUNT_READ_ONLY, 1u, base_mount ), fs::fs_error_t::OK, "mount base overlay" ) ) {
        return 1;
    }
    fs::mount_handle_t mod_mount = fs::CYPHER_FILESYSTEM_INVALID_MOUNT;
    if ( !CheckError( fs::CypherFileSystem_MountDirectoryWithHandle( "game", mod_content_path.string().c_str(), fs::CYPHER_FILESYSTEM_MOUNT_READ_ONLY, 2u, mod_mount ), fs::fs_error_t::OK, "mount mod overlay" ) ) {
        return 1;
    }

    if ( !CheckError( fs::CypherFileSystem_ResolvePath( "game/shared.cfg", path_buffer, sizeof( path_buffer ) ), fs::fs_error_t::OK, "resolve overlay priority" ) ) {
        return 1;
    }
    if ( !Check( std::filesystem::equivalent( std::filesystem::path( path_buffer ), mod_content_path / "shared.cfg", ec ), "overlay priority chooses highest mount" ) ) {
        return 1;
    }
    if ( !Check( path_buffer[0] != '\0', "resolve loose overlay returns physical path" ) ) {
        return 1;
    }

    fs::directory_entry_t overlay_entries[8]{};
    cypher::engine::common::u32 overlay_entry_count = 0u;
    if ( !CheckError( fs::CypherFileSystem_ListDirectory( "game", overlay_entries, 8u, overlay_entry_count ), fs::fs_error_t::OK, "list overlay directory" ) ) {
        return 1;
    }
    if ( !Check( overlay_entry_count == 3u, "overlay list suppresses duplicate names" ) ) {
        return 1;
    }
    if ( !Check( HasDirectoryEntry( overlay_entries, overlay_entry_count, "game/shared.cfg" ), "overlay list has shared file" ) ) {
        return 1;
    }
    if ( !Check( HasDirectoryEntry( overlay_entries, overlay_entry_count, "game/base_only.cfg" ), "overlay list has base-only file" ) ) {
        return 1;
    }
    if ( !Check( HasDirectoryEntry( overlay_entries, overlay_entry_count, "game/mod_only.cfg" ), "overlay list has mod-only file" ) ) {
        return 1;
    }

    if ( !CheckError( fs::CypherFileSystem_Unmount( mod_mount ), fs::fs_error_t::OK, "unmount overlay by handle" ) ) {
        return 1;
    }
    if ( !CheckError( fs::CypherFileSystem_GetMountInfoByHandle( mod_mount, mount_info ), fs::fs_error_t::ERR_MOUNT_NOT_FOUND, "unmounted handle not found" ) ) {
        return 1;
    }
    if ( !CheckError( fs::CypherFileSystem_ResolvePath( "game/shared.cfg", path_buffer, sizeof( path_buffer ) ), fs::fs_error_t::OK, "resolve overlay after unmount" ) ) {
        return 1;
    }
    if ( !Check( std::filesystem::equivalent( std::filesystem::path( path_buffer ), base_content_path / "shared.cfg", ec ), "overlay falls back after unmount" ) ) {
        return 1;
    }

    const std::filesystem::path package_source_path = root_path / "PackageSource";
    const std::filesystem::path package_shared_path = package_source_path / "Shared.cfg";
    const std::filesystem::path package_only_path = package_source_path / "PackageOnly.cfg";
    const std::filesystem::path package_script_path = package_source_path / "Init.cfg";
    const std::filesystem::path package_archive_path = root_path / "game.cypak";
    const std::string package_shared_path_string = package_shared_path.string();
    const std::string package_only_path_string = package_only_path.string();
    const std::string package_script_path_string = package_script_path.string();
    const std::string package_archive_path_string = package_archive_path.string();
    const char package_shared_text[] = "package\n";
    const char package_only_text[] = "package_only\n";
    const char package_script_text[] = "exec autoexec\n";

    if ( !Check( WritePhysicalTextFile( package_shared_path, package_shared_text ), "write package shared source" ) ) {
        return 1;
    }
    if ( !Check( WritePhysicalTextFile( package_only_path, package_only_text ), "write package-only source" ) ) {
        return 1;
    }
    if ( !Check( WritePhysicalTextFile( package_script_path, package_script_text ), "write package script source" ) ) {
        return 1;
    }

    const pak::pak_source_file_t package_files[] = {
        { "shared.cfg", package_shared_path_string.c_str(), pak::pak_compression_t::NONE, pak::CYPHER_PAK_ENTRY_NONE },
        { "package_only.cfg", package_only_path_string.c_str(), pak::pak_compression_t::NONE, pak::CYPHER_PAK_ENTRY_NONE },
        { "scripts/init.cfg", package_script_path_string.c_str(), pak::pak_compression_t::NONE, pak::CYPHER_PAK_ENTRY_NONE }
    };
    pak::pak_writer_config_t package_config{};
    package_config.archive_path = package_archive_path_string.c_str();
    if ( pak::CypherPak_CreateArchive( package_config, package_files, 3u ) != pak::pak_error_t::OK ) {
        std::fprintf( stderr, "filesystem smoke failed: create package archive\n" );
        return 1;
    }

    if ( !CheckError( fs::CypherFileSystem_MountPackage( "game", package_archive_path_string.c_str(), fs::CYPHER_FILESYSTEM_MOUNT_READ_ONLY, 3u ), fs::fs_error_t::OK, "mount package overlay" ) ) {
        return 1;
    }
    if ( !Check( fs::CypherFileSystem_PackageIsMounted( package_archive_path_string.c_str() ), "package is mounted" ) ) {
        return 1;
    }
    fs::package_info_t package_info{};
    if ( !CheckError( fs::CypherFileSystem_GetPackageInfo( package_archive_path_string.c_str(), package_info ), fs::fs_error_t::OK, "get package info" ) ) {
        return 1;
    }
    if ( !Check( package_info.mounted && package_info.file_count == 3u && package_info.priority == 3u, "package info values" ) ) {
        return 1;
    }

    if ( !CheckError( fs::CypherFileSystem_ResolvePath( "game/shared.cfg", path_buffer, sizeof( path_buffer ) ), fs::fs_error_t::ERR_UNSUPPORTED_BACKEND, "resolve package overlay unsupported" ) ) {
        return 1;
    }
    if ( !Check( path_buffer[0] == '\0', "resolve package overlay has no physical path" ) ) {
        return 1;
    }

    char package_read_buffer[64]{};
    cypher::engine::common::u64 package_bytes_read = 0u;
    if ( !CheckError( fs::CypherFileSystem_ReadEntireFile( "game/shared.cfg", package_read_buffer, sizeof( package_read_buffer ), package_bytes_read ), fs::fs_error_t::OK, "read package overlay file" ) ) {
        return 1;
    }
    if ( !Check( package_bytes_read == std::strlen( package_shared_text ) && std::memcmp( package_read_buffer, package_shared_text, std::strlen( package_shared_text ) ) == 0, "package overlay wins read" ) ) {
        return 1;
    }

    fs::file_t package_file{};
    if ( !CheckError( fs::CypherFileSystem_Open( "game/shared.cfg", fs::open_mode_t::READ_BINARY, package_file ), fs::fs_error_t::OK, "open package file" ) ) {
        return 1;
    }
    if ( !Check( package_file.backend == fs::file_backend_t::PACKAGE_FILE && package_file.readable && !package_file.writable && package_file.size == std::strlen( package_shared_text ), "package file handle values" ) ) {
        return 1;
    }
    cypher::engine::common::u64 package_position = 1u;
    if ( !CheckError( fs::CypherFileSystem_Tell( package_file, package_position ), fs::fs_error_t::OK, "tell package file start" ) ) {
        return 1;
    }
    if ( !Check( package_position == 0u, "package file starts at zero" ) ) {
        return 1;
    }
    char package_handle_buffer[16]{};
    if ( !CheckError( fs::CypherFileSystem_Read( package_file, package_handle_buffer, 4u, package_bytes_read ), fs::fs_error_t::OK, "read package file prefix" ) ) {
        return 1;
    }
    if ( !Check( package_bytes_read == 4u && std::memcmp( package_handle_buffer, "pack", 4u ) == 0, "package file prefix bytes" ) ) {
        return 1;
    }
    if ( !CheckError( fs::CypherFileSystem_Tell( package_file, package_position ), fs::fs_error_t::OK, "tell package file after read" ) ) {
        return 1;
    }
    if ( !Check( package_position == 4u, "package file cursor advances" ) ) {
        return 1;
    }
    if ( !CheckError( fs::CypherFileSystem_Seek( package_file, 5, fs::seek_origin_t::CYPHER_FILESYSTEM_SEEK_START ), fs::fs_error_t::OK, "seek package file near end" ) ) {
        return 1;
    }
    if ( !CheckError( fs::CypherFileSystem_Read( package_file, package_handle_buffer, sizeof( package_handle_buffer ), package_bytes_read ), fs::fs_error_t::OK, "read package file short at eof" ) ) {
        return 1;
    }
    if ( !Check( package_bytes_read == std::strlen( package_shared_text ) - 5u && std::memcmp( package_handle_buffer, package_shared_text + 5u, static_cast<std::size_t>( package_bytes_read ) ) == 0, "package file short eof read" ) ) {
        return 1;
    }
    if ( !CheckError( fs::CypherFileSystem_Tell( package_file, package_position ), fs::fs_error_t::OK, "tell package file eof" ) ) {
        return 1;
    }
    if ( !Check( package_position == std::strlen( package_shared_text ), "package file cursor reaches eof" ) ) {
        return 1;
    }
    package_handle_buffer[0] = 'x';
    if ( !CheckError( fs::CypherFileSystem_Read( package_file, package_handle_buffer, sizeof( package_handle_buffer ), package_bytes_read ), fs::fs_error_t::OK, "read package file at eof" ) ) {
        return 1;
    }
    if ( !Check( package_bytes_read == 0u && package_handle_buffer[0] == 'x', "package file eof read returns zero bytes" ) ) {
        return 1;
    }
    if ( !CheckError( fs::CypherFileSystem_Read( package_file, nullptr, 0u, package_bytes_read ), fs::fs_error_t::OK, "zero read package file" ) ) {
        return 1;
    }
    if ( !Check( package_bytes_read == 0u, "package file zero read count" ) ) {
        return 1;
    }
    cypher::engine::common::u64 package_bytes_written = 1u;
    if ( !CheckError( fs::CypherFileSystem_Write( package_file, "x", 1u, package_bytes_written ), fs::fs_error_t::ERR_PERMISSION_DENIED, "write package file denied" ) ) {
        return 1;
    }
    if ( !Check( package_bytes_written == 0u, "package file denied write count" ) ) {
        return 1;
    }
    if ( !CheckError( fs::CypherFileSystem_Flush( package_file ), fs::fs_error_t::ERR_PERMISSION_DENIED, "flush package file denied" ) ) {
        return 1;
    }
    if ( !CheckError( fs::CypherFileSystem_Close( package_file ), fs::fs_error_t::OK, "close package file" ) ) {
        return 1;
    }

    fs::file_info_t package_file_info{};
    if ( !CheckError( fs::CypherFileSystem_GetFileInfo( "game/shared.cfg", package_file_info ), fs::fs_error_t::OK, "package file info" ) ) {
        return 1;
    }
    if ( !Check( package_file_info.exists && package_file_info.is_package_file && package_file_info.backend == fs::file_backend_t::PACKAGE_FILE, "package file info values" ) ) {
        return 1;
    }
    if ( !Check( fs::CypherFileSystem_DirectoryExists( "game/scripts" ), "package virtual directory exists" ) ) {
        return 1;
    }

    if ( !CheckError( fs::CypherFileSystem_ListDirectory( "game", overlay_entries, 8u, overlay_entry_count ), fs::fs_error_t::OK, "list package overlay directory" ) ) {
        return 1;
    }
    if ( !Check( overlay_entry_count == 4u, "package overlay list values" ) ) {
        return 1;
    }
    if ( !Check( HasDirectoryEntry( overlay_entries, overlay_entry_count, "game/package_only.cfg" ), "package list has package-only file" ) ) {
        return 1;
    }
    if ( !Check( HasDirectoryEntry( overlay_entries, overlay_entry_count, "game/scripts" ), "package list has scripts directory" ) ) {
        return 1;
    }

    fs::directory_entry_t package_find_entries[8]{};
    cypher::engine::common::u32 package_find_count = 0u;
    if ( !CheckError( fs::CypherFileSystem_FindFiles( "game", "*.cfg", fs::CYPHER_FILESYSTEM_FIND_RECURSIVE | fs::CYPHER_FILESYSTEM_FIND_FILES | fs::CYPHER_FILESYSTEM_FIND_SORT_BY_NAME, package_find_entries, 8u, package_find_count ), fs::fs_error_t::OK, "find files through package overlay" ) ) {
        return 1;
    }
    if ( !Check( package_find_count == 4u, "package recursive find count" ) ) {
        return 1;
    }

    if ( !CheckError( fs::CypherFileSystem_CopyFile( "game/package_only.cfg", "profiles/player1/package-copy.cfg" ), fs::fs_error_t::OK, "copy package file to write path" ) ) {
        return 1;
    }
    if ( !Check( fs::CypherFileSystem_FileExists( "profiles/player1/package-copy.cfg" ), "copied package file exists" ) ) {
        return 1;
    }
    if ( !CheckError( fs::CypherFileSystem_DeleteFile( "profiles/player1/package-copy.cfg" ), fs::fs_error_t::OK, "delete copied package file" ) ) {
        return 1;
    }

    if ( !CheckError( fs::CypherFileSystem_UnmountPackage( package_archive_path_string.c_str() ), fs::fs_error_t::OK, "unmount package" ) ) {
        return 1;
    }
    if ( !Check( !fs::CypherFileSystem_PackageIsMounted( package_archive_path_string.c_str() ), "package is unmounted" ) ) {
        return 1;
    }
    package_read_buffer[0] = '\0';
    if ( !CheckError( fs::CypherFileSystem_ReadEntireFile( "game/shared.cfg", package_read_buffer, sizeof( package_read_buffer ), package_bytes_read ), fs::fs_error_t::OK, "read loose fallback after package unmount" ) ) {
        return 1;
    }
    if ( !Check( package_bytes_read == 5u && std::memcmp( package_read_buffer, "base\n", 5u ) == 0, "loose fallback after package unmount" ) ) {
        return 1;
    }

    if ( !Check( fs::CypherFileSystem_Exists( "profiles/player1/config.cfg" ), "file exists through mount" ) ) {
        return 1;
    }
    if ( !Check( fs::CypherFileSystem_FileExists( "profiles/player1/config.cfg" ), "file exists as file" ) ) {
        return 1;
    }
    if ( !Check( fs::CypherFileSystem_DirectoryExists( "profiles/player1" ), "directory exists through mount" ) ) {
        return 1;
    }

    fs::resolve_trace_t trace{};
    if ( !CheckError( fs::CypherFileSystem_TraceResolve( "profiles/player1/config.cfg", trace ), fs::fs_error_t::OK, "trace resolve" ) ) {
        return 1;
    }
    if ( !Check( TraceResolvedThroughMount( trace, read_mount ), "trace resolve values" ) ) {
        return 1;
    }

    fs::file_info_t info{};
    if ( !CheckError( fs::CypherFileSystem_GetFileInfo( "profiles/player1/config.cfg", info ), fs::fs_error_t::OK, "get file info" ) ) {
        return 1;
    }
    if ( !Check( info.exists && !info.is_directory && info.file_size == std::strlen( text ), "file info values" ) ) {
        return 1;
    }

    fs::directory_entry_t entries[8]{};
    cypher::engine::common::u32 entry_count = 0u;
    if ( !CheckError( fs::CypherFileSystem_ListDirectory( "profiles/player1", entries, 8u, entry_count ), fs::fs_error_t::OK, "list directory" ) ) {
        return 1;
    }
    if ( !Check( entry_count == 1u && std::strcmp( entries[0].name, "config.cfg" ) == 0, "list directory values" ) ) {
        return 1;
    }

    if ( !CheckError( fs::CypherFileSystem_FindFiles( "profiles", "*.cfg", fs::CYPHER_FILESYSTEM_FIND_RECURSIVE | fs::CYPHER_FILESYSTEM_FIND_FILES | fs::CYPHER_FILESYSTEM_FIND_SORT_BY_NAME, entries, 8u, entry_count ), fs::fs_error_t::OK, "find files" ) ) {
        return 1;
    }
    if ( !Check( entry_count == 1u && std::strcmp( entries[0].virtual_path, "profiles/player1/config.cfg" ) == 0, "find files values" ) ) {
        return 1;
    }

    if ( !CheckError( fs::CypherFileSystem_CreateDirectory( "bulk" ), fs::fs_error_t::OK, "create bulk directory" ) ) {
        return 1;
    }
    for ( cypher::engine::common::u32 i = 0u; i < 270u; ++i ) {
        char bulk_file[64]{};
        std::snprintf( bulk_file, sizeof( bulk_file ), "bulk/file%03u.tmp", i );
        if ( !CheckError( fs::CypherFileSystem_WriteEntireFile( bulk_file, text, std::strlen( text ) ), fs::fs_error_t::OK, "write bulk file" ) ) {
            return 1;
        }
    }

    fs::directory_entry_t bulk_entries[300]{};
    cypher::engine::common::u32 bulk_entry_count = 0u;
    if ( !CheckError( fs::CypherFileSystem_FindFiles( "bulk", "*.tmp", fs::CYPHER_FILESYSTEM_FIND_FILES, bulk_entries, 300u, bulk_entry_count ), fs::fs_error_t::OK, "find bulk files" ) ) {
        return 1;
    }
    if ( !Check( bulk_entry_count == 270u, "find files sees directories larger than scratch buffer" ) ) {
        return 1;
    }

    const char extra_text[] = "rate 1\n";
    if ( !CheckError( fs::CypherFileSystem_AppendEntireFile( "profiles/player1/config.cfg", extra_text, std::strlen( extra_text ) ), fs::fs_error_t::OK, "append entire file" ) ) {
        return 1;
    }

    char read_buffer[64]{};
    cypher::engine::common::u64 bytes_read = 0u;
    if ( !CheckError( fs::CypherFileSystem_ReadEntireFile( "profiles/player1/config.cfg", read_buffer, sizeof( read_buffer ), bytes_read ), fs::fs_error_t::OK, "read entire file" ) ) {
        return 1;
    }
    if ( !Check( bytes_read == std::strlen( text ) + std::strlen( extra_text ), "read entire byte count" ) ) {
        return 1;
    }

    if ( !CheckError( fs::CypherFileSystem_CopyFile( "profiles/player1/config.cfg", "profiles/player1/config-copy.cfg" ), fs::fs_error_t::OK, "copy file" ) ) {
        return 1;
    }
    if ( !Check( fs::CypherFileSystem_FileExists( "profiles/player1/config-copy.cfg" ), "copied file exists" ) ) {
        return 1;
    }
    if ( !CheckError( fs::CypherFileSystem_DeleteFile( "profiles/player1/config-copy.cfg" ), fs::fs_error_t::OK, "delete copied file" ) ) {
        return 1;
    }

    if ( !CheckError( fs::CypherFileSystem_CreateDirectory( "profiles/player2/nested" ), fs::fs_error_t::OK, "create recursive directory" ) ) {
        return 1;
    }
    if ( !CheckError( fs::CypherFileSystem_WriteEntireFile( "profiles/player2/nested/temp.cfg", text, std::strlen( text ) ), fs::fs_error_t::OK, "write entire file" ) ) {
        return 1;
    }
    if ( !CheckError( fs::CypherFileSystem_RemoveDirectoryTree( "profiles/player2" ), fs::fs_error_t::OK, "remove directory tree" ) ) {
        return 1;
    }
    if ( !CheckError( fs::CypherFileSystem_RemoveDirectoryTree( "bulk" ), fs::fs_error_t::OK, "remove bulk directory" ) ) {
        return 1;
    }

    fs::stats_t stats{};
    if ( !CheckError( fs::CypherFileSystem_GetStats( stats ), fs::fs_error_t::OK, "get stats" ) ) {
        return 1;
    }
    if ( !Check( stats.open_count > 0u && stats.close_count > 0u && stats.bytes_written > 0u, "stats values" ) ) {
        return 1;
    }

    fs::async_request_t request = fs::CYPHER_FILESYSTEM_INVALID_ASYNC_REQUEST;
    if ( !CheckError( fs::CypherFileSystem_ReadAsync( "profiles/player1/config.cfg", read_buffer, sizeof( read_buffer ), request ), fs::fs_error_t::ERR_NOT_IMPLEMENTED, "read async not implemented" ) ) {
        return 1;
    }
    fs::watch_event_t events[4]{};
    if ( !CheckError( fs::CypherFileSystem_PollChanges( events, 4u, entry_count ), fs::fs_error_t::ERR_NOT_IMPLEMENTED, "poll changes not implemented" ) ) {
        return 1;
    }
    if ( !CheckError( fs::CypherFileSystem_MountPackage( "", "pak0.pak", fs::CYPHER_FILESYSTEM_MOUNT_READ_ONLY | fs::CYPHER_FILESYSTEM_MOUNT_OPTIONAL, 0u ), fs::fs_error_t::OK, "optional missing package mount" ) ) {
        return 1;
    }

    if ( !CheckError( fs::CypherFileSystem_Rename( "profiles/player1/config.cfg", "profiles/player1/config2.cfg" ), fs::fs_error_t::OK, "rename file" ) ) {
        return 1;
    }
    if ( !Check( fs::CypherFileSystem_Exists( "profiles/player1/config2.cfg" ), "renamed file exists through mount" ) ) {
        return 1;
    }
    if ( !CheckError( fs::CypherFileSystem_DeleteFile( "profiles/player1/config2.cfg" ), fs::fs_error_t::OK, "delete file" ) ) {
        return 1;
    }
    if ( !CheckError( fs::CypherFileSystem_RemoveDirectory( "profiles/player1" ), fs::fs_error_t::OK, "remove player directory" ) ) {
        return 1;
    }
    if ( !CheckError( fs::CypherFileSystem_RemoveDirectory( "profiles" ), fs::fs_error_t::OK, "remove profiles directory" ) ) {
        return 1;
    }
    if ( !CheckError( fs::CypherFileSystem_Unmount( base_mount ), fs::fs_error_t::OK, "unmount base overlay" ) ) {
        return 1;
    }
    if ( !CheckError( fs::CypherFileSystem_Unmount( read_mount ), fs::fs_error_t::OK, "unmount by handle" ) ) {
        return 1;
    }
    if ( !CheckError( fs::CypherFileSystem_Shutdown(), fs::fs_error_t::OK, "shutdown" ) ) {
        return 1;
    }

    std::filesystem::remove_all( root_path, ec );
    return 0;
}

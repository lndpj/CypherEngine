#include "CypherEngine/CypherFileSystem/CypherFileSystem_Runtime.h"
#include "CypherEngine/CypherLog/CypherLog.h"
#include "CypherEngine/CypherPak/CypherPak.h"

#include <cstring>
#include <filesystem>
#include <new>
#include <system_error>

namespace cypher::engine::fs
{

namespace {

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
    case pak::pak_error_t::ERR_NOT_IMPLEMENTED:
        return fs_error_t::ERR_NOT_IMPLEMENTED;
    case pak::pak_error_t::ERR_UNSUPPORTED_COMPRESSION:
        return fs_error_t::ERR_UNSUPPORTED_BACKEND;
    default:
        return fs_error_t::ERR_IO_ERROR;
    }
}

bool CopyString( char *out, const common::u32 out_size, const char *text )
{
    if ( out == nullptr || out_size == 0u || text == nullptr ) {
        return false;
    }

    const common::usize len = std::strlen( text );
    if ( len + 1u > out_size ) {
        out[0] = '\0';
        return false;
    }

    std::memcpy( out, text, len + 1u );
    return true;
}

pak::pak_reader_t *PackageReader( const mount_t &mount )
{
    return static_cast<pak::pak_reader_t *>( mount.package_reader );
}

}       // namespace

fs_error_t CypherFileSystem_MountPackage(
    const char *virtual_root,
    const char *package_path,
    common::u32 flags,
    common::u32 priority )
{
    runtime_state_t &state = CypherFileSystem_RuntimeState();
    std::lock_guard<std::recursive_mutex> lock( CypherFileSystem_RuntimeMutex() );

    if ( !state.initialized ) {
        return fs_error_t::ERR_NOT_INIT;
    }
    if ( package_path == nullptr || package_path[0] == '\0' ) {
        return fs_error_t::ERR_INVALID_PATH;
    }

    char normalized_virtual_root[CYPHER_FILESYSTEM_MAX_VIRTUAL_ROOT_LENGTH]{};
    const fs_error_t root_result = CypherFileSystem_NormalizeVirtualRoot( virtual_root, normalized_virtual_root, sizeof( normalized_virtual_root ) );
    if ( root_result != fs_error_t::OK ) {
        return root_result;
    }

    const common::u32 allowed_flags = CYPHER_FILESYSTEM_MOUNT_READ_ONLY | CYPHER_FILESYSTEM_MOUNT_OPTIONAL;
    if ( ( flags & ~allowed_flags ) != 0u ) {
        return fs_error_t::ERR_INVALID_ARGUMENT;
    }
    if ( ( flags & CYPHER_FILESYSTEM_MOUNT_READ_ONLY ) == 0u ) {
        return fs_error_t::ERR_INVALID_ARGUMENT;
    }
    if ( state.mount_count >= CYPHER_FILESYSTEM_MAX_MOUNTS ) {
        return fs_error_t::ERR_TOO_MANY_MOUNTS;
    }

    const common::u32 package_path_len = static_cast<common::u32>( std::strlen( package_path ) );
    if ( package_path_len + 1u > CYPHER_FILESYSTEM_MAX_PATH_LENGTH ) {
        return fs_error_t::ERR_BUFFER_TOO_SMALL;
    }

    std::error_code ec{};
    if ( !std::filesystem::exists( package_path, ec ) ) {
        if ( !ec && ( flags & CYPHER_FILESYSTEM_MOUNT_OPTIONAL ) != 0u ) {
            LOG_WARNING( log::channel_t::FS, "optional package mount skipped: '%s' does not exist.", package_path );
            return fs_error_t::OK;
        }
        return ec ? fs_error_t::ERR_IO_ERROR : fs_error_t::ERR_PATH_NOT_FOUND;
    }
    if ( !std::filesystem::is_regular_file( package_path, ec ) || ec ) {
        return ec ? fs_error_t::ERR_IO_ERROR : fs_error_t::ERR_NOT_FILE;
    }

    for ( common::u32 i = 0u; i < state.mount_count; ++i ) {
        const mount_t &existing_mount = state.mounts[i];
        if ( existing_mount.type == mount_type_t::CYPHER_FILESYSTEM_PACKAGE &&
             std::strcmp( existing_mount.virtual_root, normalized_virtual_root ) == 0 &&
             std::strcmp( existing_mount.physical_root, package_path ) == 0 ) {
            return fs_error_t::ERR_ALREADY_EXISTS;
        }
    }

    pak::pak_reader_t *reader = new ( std::nothrow ) pak::pak_reader_t();
    if ( reader == nullptr ) {
        return fs_error_t::ERR_OUT_OF_MEMORY;
    }

    const pak::pak_error_t open_result = pak::CypherPak_OpenReader(
        package_path,
        pak::CYPHER_PAK_OPEN_VERIFY_INDEX,
        *reader );
    if ( open_result != pak::pak_error_t::OK ) {
        delete reader;
        return PakErrorToFs( open_result );
    }

    mount_t mount{};
    mount.handle = CypherFileSystem_AllocateMountHandle( state );
    mount.type = mount_type_t::CYPHER_FILESYSTEM_PACKAGE;
    mount.flags = flags;
    mount.priority = priority;
    mount.package_reader = reader;
    std::memcpy( mount.virtual_root, normalized_virtual_root, std::strlen( normalized_virtual_root ) + 1u );
    std::memcpy( mount.physical_root, package_path, package_path_len + 1u );

    const fs_error_t insert_result = CypherFileSystem_InsertMountByPriority( state, mount );
    if ( insert_result != fs_error_t::OK ) {
        pak::CypherPak_CloseReader( *reader );
        delete reader;
        return insert_result;
    }

    LOG_INFO( log::channel_t::FS, "mounted package '%s' -> '%s' handle=%u flags=0x%x priority=%u.", normalized_virtual_root[0] ? normalized_virtual_root : "<root>", package_path, mount.handle, flags, priority );
    return fs_error_t::OK;
}

fs_error_t CypherFileSystem_UnmountPackage( const char *package_path )
{
    runtime_state_t &state = CypherFileSystem_RuntimeState();
    std::lock_guard<std::recursive_mutex> lock( CypherFileSystem_RuntimeMutex() );
    if ( !state.initialized ) {
        return fs_error_t::ERR_NOT_INIT;
    }
    if ( package_path == nullptr || package_path[0] == '\0' ) {
        return fs_error_t::ERR_INVALID_PATH;
    }

    for ( common::u32 i = 0u; i < state.mount_count; ++i ) {
        const mount_t &mount = state.mounts[i];
        if ( mount.type == mount_type_t::CYPHER_FILESYSTEM_PACKAGE &&
             std::strcmp( mount.physical_root, package_path ) == 0 ) {
            CypherFileSystem_RemoveMountAtIndex( state, i );
            return fs_error_t::OK;
        }
    }

    return fs_error_t::ERR_MOUNT_NOT_FOUND;
}

fs_error_t CypherFileSystem_GetPackageInfo( const char *package_path, package_info_t &out_info )
{
    runtime_state_t &state = CypherFileSystem_RuntimeState();
    std::lock_guard<std::recursive_mutex> lock( CypherFileSystem_RuntimeMutex() );
    out_info = {};

    if ( !state.initialized ) {
        return fs_error_t::ERR_NOT_INIT;
    }
    if ( package_path == nullptr || package_path[0] == '\0' ) {
        return fs_error_t::ERR_INVALID_PATH;
    }

    for ( common::u32 i = 0u; i < state.mount_count; ++i ) {
        const mount_t &mount = state.mounts[i];
        if ( mount.type != mount_type_t::CYPHER_FILESYSTEM_PACKAGE ||
             std::strcmp( mount.physical_root, package_path ) != 0 ) {
            continue;
        }

        if ( !CopyString( out_info.virtual_root, sizeof( out_info.virtual_root ), mount.virtual_root ) ||
             !CopyString( out_info.package_path, sizeof( out_info.package_path ), mount.physical_root ) ) {
            out_info = {};
            return fs_error_t::ERR_BUFFER_TOO_SMALL;
        }

        common::u32 file_count = 0u;
        pak::pak_reader_t *reader = PackageReader( mount );
        const pak::pak_error_t count_result = pak::CypherPak_GetFileCount( *reader, file_count );
        if ( count_result != pak::pak_error_t::OK ) {
            out_info = {};
            return PakErrorToFs( count_result );
        }

        out_info.file_count = file_count;
        out_info.priority = mount.priority;
        out_info.mounted = true;
        return fs_error_t::OK;
    }

    if ( !CopyString( out_info.package_path, sizeof( out_info.package_path ), package_path ) ) {
        return fs_error_t::ERR_BUFFER_TOO_SMALL;
    }
    out_info.mounted = false;
    return fs_error_t::ERR_MOUNT_NOT_FOUND;
}

bool CypherFileSystem_PackageIsMounted( const char *package_path )
{
    std::lock_guard<std::recursive_mutex> lock( CypherFileSystem_RuntimeMutex() );
    if ( !CypherFileSystem_RuntimeState().initialized || package_path == nullptr || package_path[0] == '\0' ) {
        return false;
    }

    const runtime_state_t &state = CypherFileSystem_RuntimeState();
    for ( common::u32 i = 0u; i < state.mount_count; ++i ) {
        const mount_t &mount = state.mounts[i];
        if ( mount.type == mount_type_t::CYPHER_FILESYSTEM_PACKAGE &&
             std::strcmp( mount.physical_root, package_path ) == 0 ) {
            return true;
        }
    }

    return false;
}

}       // namespace cypher::engine::fs

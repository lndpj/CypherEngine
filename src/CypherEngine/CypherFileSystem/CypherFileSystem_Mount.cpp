#include "CypherEngine/CypherFileSystem/CypherFileSystem_Runtime.h"
#include "CypherEngine/CypherLog/CypherLog.h"
#include "CypherEngine/CypherPak/CypherPak.h"

#include <cstring>
#include <filesystem>
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

pak::pak_reader_t *PackageReader( const mount_t &mount )
{
    return static_cast<pak::pak_reader_t *>( mount.package_reader );
}

void FillMountInfo( const mount_t &mount, mount_info_t &out_info )
{
    out_info = {};
    out_info.handle = mount.handle;
    out_info.type = mount.type;
    out_info.flags = mount.flags;
    out_info.priority = mount.priority;
    std::memcpy( out_info.virtual_root, mount.virtual_root, std::strlen( mount.virtual_root ) + 1u );
    std::memcpy( out_info.physical_root, mount.physical_root, std::strlen( mount.physical_root ) + 1u );
}

}       // namespace

mount_handle_t CypherFileSystem_AllocateMountHandle( runtime_state_t &state )
{
    mount_handle_t handle = state.next_mount_handle++;
    if ( handle == CYPHER_FILESYSTEM_INVALID_MOUNT ) {
        handle = state.next_mount_handle++;
    }
    return handle;
}

fs_error_t CypherFileSystem_InsertMountByPriority( runtime_state_t &state, const mount_t &mount )
{
    if ( state.mount_count >= CYPHER_FILESYSTEM_MAX_MOUNTS ) {
        return fs_error_t::ERR_TOO_MANY_MOUNTS;
    }

    common::u32 insert_index = state.mount_count;
    while ( insert_index > 0u && state.mounts[insert_index - 1u].priority < mount.priority ) {
        state.mounts[insert_index] = state.mounts[insert_index - 1u];
        --insert_index;
    }
    state.mounts[insert_index] = mount;
    ++state.mount_count;
    return fs_error_t::OK;
}

void CypherFileSystem_RemoveMountAtIndex( runtime_state_t &state, const common::u32 index )
{
    if ( index >= state.mount_count ) {
        return;
    }

    mount_t &mount = state.mounts[index];
    if ( mount.type == mount_type_t::CYPHER_FILESYSTEM_PACKAGE && mount.package_reader != nullptr ) {
        pak::pak_reader_t *reader = PackageReader( mount );
        pak::CypherPak_CloseReader( *reader );
        delete reader;
        mount.package_reader = nullptr;
    }

    for ( common::u32 j = index; j + 1u < state.mount_count; ++j ) {
        state.mounts[j] = state.mounts[j + 1u];
    }
    --state.mount_count;
    state.mounts[state.mount_count] = {};
}

common::u32 CypherFileSystem_MountCount()
{
    std::lock_guard<std::recursive_mutex> lock( CypherFileSystem_RuntimeMutex() );
    return CypherFileSystem_RuntimeState().mount_count;
}

fs_error_t CypherFileSystem_MountDirectory( const char *virtual_root, const char *physical_path, common::u32 flags, common::u32 priority )
{
    mount_handle_t ignored_handle = CYPHER_FILESYSTEM_INVALID_MOUNT;
    return CypherFileSystem_MountDirectoryWithHandle( virtual_root, physical_path, flags, priority, ignored_handle );
}

fs_error_t CypherFileSystem_MountDirectoryWithHandle(
    const char *virtual_root,
    const char *physical_path,
    common::u32 flags,
    common::u32 priority,
    mount_handle_t &out_handle )
{
    runtime_state_t &state = CypherFileSystem_RuntimeState();
    std::lock_guard<std::recursive_mutex> lock( CypherFileSystem_RuntimeMutex() );
    out_handle = CYPHER_FILESYSTEM_INVALID_MOUNT;

    if ( !state.initialized ) {
        LOG_ERROR( log::channel_t::FS, "mount failed: filesystem is not initialized." );
        return fs_error_t::ERR_NOT_INIT;
    }
    if ( physical_path == nullptr || physical_path[0] == '\0' ) {
        LOG_ERROR( log::channel_t::FS, "mount failed: physical path is invalid." );
        return fs_error_t::ERR_INVALID_PATH;
    }

    char normalized_virtual_root[CYPHER_FILESYSTEM_MAX_VIRTUAL_ROOT_LENGTH]{};
    const fs_error_t root_result = CypherFileSystem_NormalizeVirtualRoot( virtual_root, normalized_virtual_root, sizeof( normalized_virtual_root ) );
    if ( root_result != fs_error_t::OK ) {
        LOG_ERROR( log::channel_t::FS, "mount failed for '%s': invalid virtual root.", physical_path );
        return root_result;
    }

    const common::u32 access_flags = CYPHER_FILESYSTEM_MOUNT_READ_ONLY | CYPHER_FILESYSTEM_MOUNT_WRITABLE;
    const common::u32 allowed_flags = access_flags | CYPHER_FILESYSTEM_MOUNT_OPTIONAL;
    if ( ( flags & ~allowed_flags ) != 0u ) {
        LOG_ERROR( log::channel_t::FS, "mount failed for '%s': invalid flags 0x%x.", physical_path, flags );
        return fs_error_t::ERR_INVALID_ARGUMENT;
    }
    if ( ( flags & access_flags ) == 0u ) {
        LOG_ERROR( log::channel_t::FS, "mount failed for '%s': no mount access flags set.", physical_path );
        return fs_error_t::ERR_INVALID_ARGUMENT;
    }
    if ( ( flags & CYPHER_FILESYSTEM_MOUNT_WRITABLE ) != 0u ) {
        LOG_ERROR( log::channel_t::FS, "mount failed for '%s': writable mounts are not implemented; use the filesystem write path.", physical_path );
        return fs_error_t::ERR_NOT_IMPLEMENTED;
    }
    if ( state.mount_count >= CYPHER_FILESYSTEM_MAX_MOUNTS ) {
        LOG_ERROR( log::channel_t::FS, "mount failed for '%s': mount table full (%u).", physical_path, CYPHER_FILESYSTEM_MAX_MOUNTS );
        return fs_error_t::ERR_TOO_MANY_MOUNTS;
    }

    const common::u32 physical_path_length = static_cast<common::u32>( std::strlen( physical_path ) );
    if ( physical_path_length + 1u > CYPHER_FILESYSTEM_MAX_PATH_LENGTH ) {
        LOG_ERROR( log::channel_t::FS, "mount failed for '%s': physical path is too long.", physical_path );
        return fs_error_t::ERR_BUFFER_TOO_SMALL;
    }

    std::error_code ec{};
    if ( !std::filesystem::exists( physical_path, ec ) ) {
        if ( !ec && ( flags & CYPHER_FILESYSTEM_MOUNT_OPTIONAL ) != 0u ) {
            LOG_WARNING( log::channel_t::FS, "optional mount skipped: '%s' does not exist.", physical_path );
            return fs_error_t::OK;
        }
        return ec ? fs_error_t::ERR_IO_ERROR : fs_error_t::ERR_PATH_NOT_FOUND;
    }
    if ( !std::filesystem::is_directory( physical_path, ec ) || ec ) {
        return ec ? fs_error_t::ERR_IO_ERROR : fs_error_t::ERR_NOT_DIRECTORY;
    }

    for ( common::u32 i = 0u; i < state.mount_count; ++i ) {
        const mount_t &existing_mount = state.mounts[i];
        if ( std::strcmp( existing_mount.virtual_root, normalized_virtual_root ) == 0 &&
             std::strcmp( existing_mount.physical_root, physical_path ) == 0 ) {
            return fs_error_t::ERR_ALREADY_EXISTS;
        }
    }

    mount_t mount{};
    mount.handle = CypherFileSystem_AllocateMountHandle( state );
    mount.type = mount_type_t::CYPHER_FILESYSTEM_DIRECTORY;

    const common::u32 virtual_root_length = static_cast<common::u32>( std::strlen( normalized_virtual_root ) );
    std::memcpy( mount.virtual_root, normalized_virtual_root, virtual_root_length + 1u );
    std::memcpy( mount.physical_root, physical_path, physical_path_length + 1u );
    mount.flags = flags;
    mount.priority = priority;

    const fs_error_t insert_result = CypherFileSystem_InsertMountByPriority( state, mount );
    if ( insert_result != fs_error_t::OK ) {
        return insert_result;
    }
    out_handle = mount.handle;

    LOG_INFO( log::channel_t::FS, "mounted '%s' -> '%s' handle=%u flags=0x%x priority=%u.", normalized_virtual_root[0] ? normalized_virtual_root : "<root>", physical_path, mount.handle, flags, priority );
    return fs_error_t::OK;
}

fs_error_t CypherFileSystem_UnmountDirectory( const char *virtual_root )
{
    runtime_state_t &state = CypherFileSystem_RuntimeState();
    std::lock_guard<std::recursive_mutex> lock( CypherFileSystem_RuntimeMutex() );

    if ( !state.initialized ) {
        return fs_error_t::ERR_NOT_INIT;
    }

    char normalized_virtual_root[CYPHER_FILESYSTEM_MAX_VIRTUAL_ROOT_LENGTH]{};
    const fs_error_t root_result = CypherFileSystem_NormalizeVirtualRoot( virtual_root, normalized_virtual_root, sizeof( normalized_virtual_root ) );
    if ( root_result != fs_error_t::OK ) {
        return root_result;
    }

    for ( common::u32 i = 0u; i < state.mount_count; ++i ) {
        if ( state.mounts[i].type == mount_type_t::CYPHER_FILESYSTEM_DIRECTORY &&
             std::strcmp( state.mounts[i].virtual_root, normalized_virtual_root ) == 0 ) {
            CypherFileSystem_RemoveMountAtIndex( state, i );
            return fs_error_t::OK;
        }
    }

    return fs_error_t::ERR_MOUNT_NOT_FOUND;
}

fs_error_t CypherFileSystem_Unmount( mount_handle_t mount )
{
    runtime_state_t &state = CypherFileSystem_RuntimeState();
    std::lock_guard<std::recursive_mutex> lock( CypherFileSystem_RuntimeMutex() );

    if ( !state.initialized ) {
        return fs_error_t::ERR_NOT_INIT;
    }
    if ( mount == CYPHER_FILESYSTEM_INVALID_MOUNT ) {
        return fs_error_t::ERR_INVALID_HANDLE;
    }

    for ( common::u32 i = 0u; i < state.mount_count; ++i ) {
        if ( state.mounts[i].handle == mount ) {
            CypherFileSystem_RemoveMountAtIndex( state, i );
            return fs_error_t::OK;
        }
    }

    return fs_error_t::ERR_MOUNT_NOT_FOUND;
}

fs_error_t CypherFileSystem_GetMountInfo( common::u32 mount_index, mount_info_t &out_info )
{
    runtime_state_t &state = CypherFileSystem_RuntimeState();
    std::lock_guard<std::recursive_mutex> lock( CypherFileSystem_RuntimeMutex() );

    out_info = {};

    if ( !state.initialized ) {
        return fs_error_t::ERR_NOT_INIT;
    }
    if ( mount_index >= state.mount_count ) {
        return fs_error_t::ERR_INVALID_ARGUMENT;
    }

    const mount_t &mount = state.mounts[mount_index];
    FillMountInfo( mount, out_info );
    return fs_error_t::OK;
}

fs_error_t CypherFileSystem_GetMountInfoByHandle( mount_handle_t mount, mount_info_t &out_info )
{
    runtime_state_t &state = CypherFileSystem_RuntimeState();
    std::lock_guard<std::recursive_mutex> lock( CypherFileSystem_RuntimeMutex() );

    out_info = {};

    if ( !state.initialized ) {
        return fs_error_t::ERR_NOT_INIT;
    }
    if ( mount == CYPHER_FILESYSTEM_INVALID_MOUNT ) {
        return fs_error_t::ERR_INVALID_HANDLE;
    }

    for ( common::u32 i = 0u; i < state.mount_count; ++i ) {
        if ( state.mounts[i].handle == mount ) {
            FillMountInfo( state.mounts[i], out_info );
            return fs_error_t::OK;
        }
    }

    return fs_error_t::ERR_MOUNT_NOT_FOUND;
}

fs_error_t CypherFileSystem_ResolvePath( const char *virtual_path, char *out_resolved_path, common::u32 out_resolved_path_size )
{
    runtime_state_t &state = CypherFileSystem_RuntimeState();
    std::lock_guard<std::recursive_mutex> lock( CypherFileSystem_RuntimeMutex() );
    if ( !state.initialized ) {
        return fs_error_t::ERR_NOT_INIT;
    }
    if ( virtual_path == nullptr || virtual_path[0] == '\0' ) {
        return fs_error_t::ERR_INVALID_PATH;
    }
    if ( out_resolved_path == nullptr || out_resolved_path_size == 0u ) {
        return fs_error_t::ERR_INVALID_ARGUMENT;
    }
    out_resolved_path[0] = '\0';
    resolved_file_t resolved_file{};
    const fs_error_t resolved_result = CypherFileSystem_ResolveReadableFile( virtual_path, resolved_file );
    if ( resolved_result != fs_error_t::OK ) {
        return resolved_result;
    }
    if ( resolved_file.backend != file_backend_t::OS_FILE ) {
        return fs_error_t::ERR_UNSUPPORTED_BACKEND;
    }
    const common::u32 resolved_path_length = static_cast<common::u32>( std::strlen( resolved_file.physical_path ) );
    if ( resolved_path_length + 1u > out_resolved_path_size ) {
        return fs_error_t::ERR_BUFFER_TOO_SMALL;
    }
    std::memcpy( out_resolved_path, resolved_file.physical_path, resolved_path_length + 1u );
    return fs_error_t::OK;
}

fs_error_t CypherFileSystem_ResolveReadableFile( const char *virtual_path, resolved_file_t &out_file )
{
    runtime_state_t &state = CypherFileSystem_RuntimeState();
    std::lock_guard<std::recursive_mutex> lock( CypherFileSystem_RuntimeMutex() );

    out_file = {};

    if ( !state.initialized ) {
        return fs_error_t::ERR_NOT_INIT;
    }
    if ( virtual_path == nullptr || virtual_path[0] == '\0' ) {
        return fs_error_t::ERR_INVALID_PATH;
    }

    const fs_error_t normalize_result = CypherFileSystem_NormalizeVirtualPath(
        virtual_path,
        out_file.normalized_path,
        sizeof( out_file.normalized_path ) );
    if ( normalize_result != fs_error_t::OK ) {
        return normalize_result;
    }

    for ( common::u32 i = 0u; i < state.mount_count; ++i ) {
        const mount_t &mount = state.mounts[i];

        const char *relative_path = nullptr;
        if ( !CypherFileSystem_VirtualPathStartsWithRoot( out_file.normalized_path, mount.virtual_root, &relative_path ) ) {
            continue;
        }
        if ( relative_path == nullptr || relative_path[0] == '\0' ) {
            continue;
        }

        if ( mount.type == mount_type_t::CYPHER_FILESYSTEM_DIRECTORY ) {
            char candidate_path[CYPHER_FILESYSTEM_MAX_PATH_LENGTH]{};
            const fs_error_t build_path_result = CypherFileSystem_BuildPhysicalPath( mount.physical_root, relative_path, candidate_path, sizeof( candidate_path ) );
            if ( build_path_result != fs_error_t::OK ) {
                return build_path_result;
            }

            std::error_code ec{};
            if ( !std::filesystem::exists( candidate_path, ec ) ) {
                continue;
            }
            if ( ec ) {
                return fs_error_t::ERR_IO_ERROR;
            }
            if ( std::filesystem::is_directory( candidate_path, ec ) || ec ) {
                if ( ec ) {
                    return fs_error_t::ERR_IO_ERROR;
                }
                continue;
            }

            out_file.backend = file_backend_t::OS_FILE;
            out_file.mount = mount.handle;
            out_file.mount_index = i;
            out_file.is_directory = false;

            const common::u32 candidate_len = static_cast<common::u32>( std::strlen( candidate_path ) );
            if ( candidate_len + 1u > sizeof( out_file.physical_path ) ) {
                return fs_error_t::ERR_BUFFER_TOO_SMALL;
            }
            std::memcpy( out_file.physical_path, candidate_path, candidate_len + 1u );

            out_file.file_size = static_cast<common::u64>( std::filesystem::file_size( candidate_path, ec ) );
            return ec ? fs_error_t::ERR_IO_ERROR : fs_error_t::OK;
        }

        if ( mount.type == mount_type_t::CYPHER_FILESYSTEM_PACKAGE ) {
            pak::pak_reader_t *reader = PackageReader( mount );
            if ( reader == nullptr ) {
                continue;
            }

            pak::pak_file_index_t package_index = pak::CYPHER_PAK_INVALID_FILE_INDEX;
            const pak::pak_error_t find_result = pak::CypherPak_FindFile( *reader, relative_path, package_index );
            if ( find_result == pak::pak_error_t::ERR_ENTRY_NOT_FOUND ) {
                continue;
            }
            if ( find_result != pak::pak_error_t::OK ) {
                return PakErrorToFs( find_result );
            }

            pak::pak_file_info_t package_info{};
            const pak::pak_error_t info_result = pak::CypherPak_GetFileInfo( *reader, package_index, package_info );
            if ( info_result != pak::pak_error_t::OK ) {
                return PakErrorToFs( info_result );
            }

            out_file.backend = file_backend_t::PACKAGE_FILE;
            out_file.mount = mount.handle;
            out_file.mount_index = i;
            out_file.package_file_index = package_index;
            out_file.package_reader = reader;
            out_file.file_size = package_info.unpacked_size;
            out_file.is_directory = false;

            const common::u32 package_path_len = static_cast<common::u32>( std::strlen( mount.physical_root ) );
            if ( package_path_len + 1u > sizeof( out_file.package_path ) ) {
                return fs_error_t::ERR_BUFFER_TOO_SMALL;
            }
            std::memcpy( out_file.package_path, mount.physical_root, package_path_len + 1u );
            return fs_error_t::OK;
        }
    }

    state.stats.failed_lookup_count++;
    return fs_error_t::ERR_PATH_NOT_FOUND;
}

fs_error_t CypherFileSystem_TraceResolve( const char *virtual_path, resolve_trace_t &out_trace )
{
    runtime_state_t &state = CypherFileSystem_RuntimeState();
    std::lock_guard<std::recursive_mutex> lock( CypherFileSystem_RuntimeMutex() );

    out_trace = resolve_trace_t{};

    if ( !state.initialized ) {
        out_trace.result = fs_error_t::ERR_NOT_INIT;
        return fs_error_t::ERR_NOT_INIT;
    }
    if ( virtual_path == nullptr || virtual_path[0] == '\0' ) {
        out_trace.result = fs_error_t::ERR_INVALID_PATH;
        return fs_error_t::ERR_INVALID_PATH;
    }

    const common::usize requested_len = std::strlen( virtual_path );
    if ( requested_len + 1u > sizeof( out_trace.requested_path ) ) {
        out_trace.result = fs_error_t::ERR_BUFFER_TOO_SMALL;
        return fs_error_t::ERR_BUFFER_TOO_SMALL;
    }
    std::memcpy( out_trace.requested_path, virtual_path, requested_len + 1u );

    const fs_error_t normalize_result = CypherFileSystem_NormalizeVirtualPath( virtual_path, out_trace.normalized_path, sizeof( out_trace.normalized_path ) );
    if ( normalize_result != fs_error_t::OK ) {
        out_trace.result = normalize_result;
        return normalize_result;
    }

    for ( common::u32 i = 0u; i < state.mount_count; ++i ) {
        const mount_t &mount = state.mounts[i];
        resolve_trace_entry_t &entry = out_trace.entries[out_trace.checked_mount_count++];
        entry.mount = mount.handle;
        entry.type = mount.type;
        std::memcpy( entry.virtual_root, mount.virtual_root, std::strlen( mount.virtual_root ) + 1u );

        const char *relative_path = nullptr;
        if ( !CypherFileSystem_VirtualPathStartsWithRoot( out_trace.normalized_path, mount.virtual_root, &relative_path ) ) {
            continue;
        }

        entry.root_matched = true;

        if ( mount.type == mount_type_t::CYPHER_FILESYSTEM_PACKAGE ) {
            pak::pak_reader_t *reader = PackageReader( mount );
            if ( reader == nullptr || relative_path == nullptr || relative_path[0] == '\0' ) {
                continue;
            }

            const common::usize package_path_len = std::strlen( mount.physical_root );
            if ( package_path_len + 1u > sizeof( entry.physical_path ) ) {
                out_trace.result = fs_error_t::ERR_BUFFER_TOO_SMALL;
                return fs_error_t::ERR_BUFFER_TOO_SMALL;
            }
            std::memcpy( entry.physical_path, mount.physical_root, package_path_len + 1u );

            pak::pak_file_index_t package_index = pak::CYPHER_PAK_INVALID_FILE_INDEX;
            const pak::pak_error_t find_result = pak::CypherPak_FindFile( *reader, relative_path, package_index );
            if ( find_result == pak::pak_error_t::ERR_ENTRY_NOT_FOUND ) {
                continue;
            }
            if ( find_result != pak::pak_error_t::OK ) {
                out_trace.result = PakErrorToFs( find_result );
                return out_trace.result;
            }

            entry.path_exists = true;
            std::memcpy( out_trace.resolved_path, entry.physical_path, package_path_len + 1u );
            out_trace.resolved = true;
            out_trace.result = fs_error_t::OK;
            return fs_error_t::OK;
        }

        if ( mount.type != mount_type_t::CYPHER_FILESYSTEM_DIRECTORY ) {
            continue;
        }

        const fs_error_t build_path_result = CypherFileSystem_BuildPhysicalPath( mount.physical_root, relative_path, entry.physical_path, sizeof( entry.physical_path ) );
        if ( build_path_result != fs_error_t::OK ) {
            out_trace.result = build_path_result;
            return build_path_result;
        }

        std::error_code ec{};
        entry.path_exists = std::filesystem::exists( entry.physical_path, ec );
        if ( ec ) {
            out_trace.result = fs_error_t::ERR_IO_ERROR;
            return fs_error_t::ERR_IO_ERROR;
        }
        if ( !entry.path_exists ) {
            continue;
        }

        const common::usize resolved_len = std::strlen( entry.physical_path );
        if ( resolved_len + 1u > sizeof( out_trace.resolved_path ) ) {
            out_trace.result = fs_error_t::ERR_BUFFER_TOO_SMALL;
            return fs_error_t::ERR_BUFFER_TOO_SMALL;
        }

        std::memcpy( out_trace.resolved_path, entry.physical_path, resolved_len + 1u );
        out_trace.resolved = true;
        out_trace.result = fs_error_t::OK;
        return fs_error_t::OK;
    }

    out_trace.result = fs_error_t::ERR_PATH_NOT_FOUND;
    return fs_error_t::ERR_PATH_NOT_FOUND;
}

bool CypherFileSystem_Exists( const char *virtual_path )
{
    std::lock_guard<std::recursive_mutex> lock( CypherFileSystem_RuntimeMutex() );
    if ( !CypherFileSystem_RuntimeState().initialized ) {
        return false;
    }

    resolved_file_t resolved_file{};
    if ( CypherFileSystem_ResolveReadableFile( virtual_path, resolved_file ) == fs_error_t::OK ) {
        return true;
    }

    common::u32 entry_count = 0u;
    const fs_error_t list_result = CypherFileSystem_ListDirectory( virtual_path, nullptr, 0u, entry_count );
    return list_result == fs_error_t::OK || list_result == fs_error_t::ERR_BUFFER_TOO_SMALL;
}

bool CypherFileSystem_FileExists( const char *virtual_path )
{
    std::lock_guard<std::recursive_mutex> lock( CypherFileSystem_RuntimeMutex() );
    if ( !CypherFileSystem_RuntimeState().initialized ) {
        return false;
    }

    file_info_t info{};
    return CypherFileSystem_GetFileInfo( virtual_path, info ) == fs_error_t::OK && info.exists && !info.is_directory;
}

fs_error_t CypherFileSystem_GetFileInfo( const char *virtual_path, file_info_t &out_info )
{
    std::lock_guard<std::recursive_mutex> lock( CypherFileSystem_RuntimeMutex() );
    if ( !CypherFileSystem_RuntimeState().initialized ) {
        return fs_error_t::ERR_NOT_INIT;
    }

    out_info = {};

    if ( virtual_path == nullptr || virtual_path[0] == '\0' ) {
        return fs_error_t::ERR_INVALID_PATH;
    }

    char normalized_path[CYPHER_FILESYSTEM_MAX_PATH_LENGTH]{};
    fs_error_t err = CypherFileSystem_NormalizeVirtualPath( virtual_path, normalized_path, sizeof( normalized_path ) );
    if ( err != fs_error_t::OK ) {
        return err;
    }

    std::memcpy( out_info.virtual_path, normalized_path, std::strlen( normalized_path ) + 1u );

    resolved_file_t resolved_file{};
    err = CypherFileSystem_ResolveReadableFile( normalized_path, resolved_file );
    if ( err == fs_error_t::OK ) {
        out_info.exists = true;
        out_info.is_directory = false;
        out_info.backend = resolved_file.backend;
        out_info.file_size = resolved_file.file_size;
        out_info.is_package_file = resolved_file.backend == file_backend_t::PACKAGE_FILE;

        if ( resolved_file.backend == file_backend_t::OS_FILE ) {
            std::memcpy( out_info.resolved_path, resolved_file.physical_path, std::strlen( resolved_file.physical_path ) + 1u );
        } else if ( resolved_file.backend == file_backend_t::PACKAGE_FILE ) {
            std::memcpy( out_info.package_path, resolved_file.package_path, std::strlen( resolved_file.package_path ) + 1u );
        }

        return fs_error_t::OK;
    }
    if ( err != fs_error_t::ERR_PATH_NOT_FOUND ) {
        return err;
    }

    common::u32 entry_count = 0u;
    err = CypherFileSystem_ListDirectory( normalized_path, nullptr, 0u, entry_count );
    if ( err != fs_error_t::OK && err != fs_error_t::ERR_BUFFER_TOO_SMALL ) {
        out_info = {};
        return err;
    }

    out_info.exists = true;
    out_info.is_directory = true;
    out_info.backend = file_backend_t::OS_FILE;
    out_info.is_package_file = false;
    return fs_error_t::OK;
}

}       // namespace cypher::engine::fs

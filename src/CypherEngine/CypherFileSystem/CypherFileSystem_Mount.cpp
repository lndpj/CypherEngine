#include "CypherEngine/CypherFileSystem/CypherFileSystem_Runtime.h"
#include "CypherEngine/CypherLog/CypherLog.h"

#include <cstring>
#include <filesystem>
#include <system_error>

namespace cypher::engine::fs
{

common::u32 CypherFileSystem_MountCount()
{
    return CypherFileSystem_RuntimeState().mount_count;
}

fs_error_t CypherFileSystem_MountDirectory( const char *virtual_root, const char *physical_path, common::u32 flags, common::u32 priority )
{
    runtime_state_t &state = CypherFileSystem_RuntimeState();

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

    const common::u32 allowed_flags = CYPHER_FILESYSTEM_MOUNT_READ_ONLY | CYPHER_FILESYSTEM_MOUNT_WRITABLE;
    if ( ( flags & ~allowed_flags ) != 0u ) {
        LOG_ERROR( log::channel_t::FS, "mount failed for '%s': invalid flags 0x%x.", physical_path, flags );
        return fs_error_t::ERR_INVALID_ARGUMENT;
    }
    if ( ( flags & allowed_flags ) == 0u ) {
        LOG_ERROR( log::channel_t::FS, "mount failed for '%s': no mount access flags set.", physical_path );
        return fs_error_t::ERR_INVALID_ARGUMENT;
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
    mount.type = mount_type_t::CYPHER_FILESYSTEM_DIRECTORY;

    const common::u32 virtual_root_length = static_cast<common::u32>( std::strlen( normalized_virtual_root ) );
    std::memcpy( mount.virtual_root, normalized_virtual_root, virtual_root_length + 1u );
    std::memcpy( mount.physical_root, physical_path, physical_path_length + 1u );
    mount.flags = flags;
    mount.priority = priority;

    common::u32 insert_index = state.mount_count;
    while ( insert_index > 0u && state.mounts[insert_index - 1u].priority < priority ) {
        state.mounts[insert_index] = state.mounts[insert_index - 1u];
        --insert_index;
    }
    state.mounts[insert_index] = mount;
    ++state.mount_count;

    LOG_INFO( log::channel_t::FS, "mounted '%s' -> '%s' flags=0x%x priority=%u.", normalized_virtual_root[0] ? normalized_virtual_root : "<root>", physical_path, flags, priority );
    return fs_error_t::OK;
}

fs_error_t CypherFileSystem_UnmountDirectory( const char *virtual_root )
{
    runtime_state_t &state = CypherFileSystem_RuntimeState();

    if ( !state.initialized ) {
        return fs_error_t::ERR_NOT_INIT;
    }

    char normalized_virtual_root[CYPHER_FILESYSTEM_MAX_VIRTUAL_ROOT_LENGTH]{};
    const fs_error_t root_result = CypherFileSystem_NormalizeVirtualRoot( virtual_root, normalized_virtual_root, sizeof( normalized_virtual_root ) );
    if ( root_result != fs_error_t::OK ) {
        return root_result;
    }

    for ( common::u32 i = 0u; i < state.mount_count; ++i ) {
        if ( std::strcmp( state.mounts[i].virtual_root, normalized_virtual_root ) == 0 ) {
            for ( common::u32 j = i; j + 1u < state.mount_count; ++j ) {
                state.mounts[j] = state.mounts[j + 1u];
            }
            --state.mount_count;
            state.mounts[state.mount_count] = {};
            return fs_error_t::OK;
        }
    }

    return fs_error_t::ERR_MOUNT_NOT_FOUND;
}

fs_error_t CypherFileSystem_ResolvePath( const char *virtual_path, char *out_resolved_path, common::u32 out_resolved_path_size )
{
    runtime_state_t &state = CypherFileSystem_RuntimeState();

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

    char normalized_path[CYPHER_FILESYSTEM_MAX_PATH_LENGTH]{};
    const fs_error_t normalize_result = CypherFileSystem_NormalizeVirtualPath( virtual_path, normalized_path, sizeof( normalized_path ) );
    if ( normalize_result != fs_error_t::OK ) {
        return normalize_result;
    }

    for ( common::u32 i = 0u; i < state.mount_count; ++i ) {
        const mount_t &mount = state.mounts[i];
        if ( mount.type != mount_type_t::CYPHER_FILESYSTEM_DIRECTORY ) {
            continue;
        }

        const char *relative_path = nullptr;
        if ( !CypherFileSystem_VirtualPathStartsWithRoot( normalized_path, mount.virtual_root, &relative_path ) ) {
            continue;
        }

        char candidate_path[CYPHER_FILESYSTEM_MAX_PATH_LENGTH]{};
        const fs_error_t build_path_result = CypherFileSystem_BuildPhysicalPath( mount.physical_root, relative_path, candidate_path, sizeof( candidate_path ) );
        if ( build_path_result != fs_error_t::OK ) {
            out_resolved_path[0] = '\0';
            return build_path_result;
        }

        std::error_code ec{};
        if ( !std::filesystem::exists( candidate_path, ec ) ) {
            continue;
        }
        if ( ec ) {
            return fs_error_t::ERR_IO_ERROR;
        }

        const common::u32 candidate_path_length = static_cast<common::u32>( std::strlen( candidate_path ) );
        if ( candidate_path_length + 1u > out_resolved_path_size ) {
            out_resolved_path[0] = '\0';
            return fs_error_t::ERR_BUFFER_TOO_SMALL;
        }

        std::memcpy( out_resolved_path, candidate_path, candidate_path_length + 1u );
        return fs_error_t::OK;
    }

    return fs_error_t::ERR_PATH_NOT_FOUND;
}

bool CypherFileSystem_Exists( const char *virtual_path )
{
    if ( !CypherFileSystem_RuntimeState().initialized ) {
        return false;
    }

    char resolved_path[CYPHER_FILESYSTEM_MAX_PATH_LENGTH]{};
    return CypherFileSystem_ResolvePath( virtual_path, resolved_path, sizeof( resolved_path ) ) == fs_error_t::OK;
}

fs_error_t CypherFileSystem_GetFileInfo( const char *virtual_path, file_info_t &out_info )
{
    if ( !CypherFileSystem_RuntimeState().initialized ) {
        return fs_error_t::ERR_NOT_INIT;
    }

    out_info = {};

    if ( virtual_path == nullptr || virtual_path[0] == '\0' ) {
        return fs_error_t::ERR_INVALID_PATH;
    }

    fs_error_t err = CypherFileSystem_ResolvePath( virtual_path, out_info.resolved_path, sizeof( out_info.resolved_path ) );
    if ( err != fs_error_t::OK ) {
        return err;
    }

    std::error_code ec{};
    out_info.exists = std::filesystem::exists( out_info.resolved_path, ec );
    if ( ec ) {
        out_info = {};
        return fs_error_t::ERR_IO_ERROR;
    }

    out_info.is_directory = std::filesystem::is_directory( out_info.resolved_path, ec );
    if ( ec ) {
        out_info = {};
        return fs_error_t::ERR_IO_ERROR;
    }

    if ( !out_info.is_directory ) {
        out_info.file_size = static_cast<common::u64>( std::filesystem::file_size( out_info.resolved_path, ec ) );
        if ( ec ) {
            out_info = {};
            return fs_error_t::ERR_IO_ERROR;
        }
    }

    return fs_error_t::OK;
}

}       // namespace cypher::engine::fs

#pragma once

#include "CypherEngine/CypherFileSystem/CypherFileSystem.h"

#include <mutex>

namespace cypher::engine::fs
{

/*
================
Filesystem Runtime

Shared state and helpers used only by filesystem implementation files.
================
*/
struct runtime_state_t {
    bool initialized{ false };
    mount_t mounts[CYPHER_FILESYSTEM_MAX_MOUNTS]{};
    common::u32 mount_count{ 0u };
    mount_handle_t next_mount_handle{ 1u };
    char write_path[CYPHER_FILESYSTEM_MAX_PATH_LENGTH]{};
    stats_t stats{};
};

struct resolved_file_t {
    file_backend_t backend{ file_backend_t::INVALID };
    mount_handle_t mount{ CYPHER_FILESYSTEM_INVALID_MOUNT };
    common::u32 mount_index{ 0u };
    common::u32 package_file_index{ 0u };
    void *package_reader{ nullptr };
    common::u64 file_size{ 0u };
    bool is_directory{ false };
    char normalized_path[CYPHER_FILESYSTEM_MAX_PATH_LENGTH]{};
    char physical_path[CYPHER_FILESYSTEM_MAX_PATH_LENGTH]{};
    char package_path[CYPHER_FILESYSTEM_MAX_PATH_LENGTH]{};
};

runtime_state_t &CypherFileSystem_RuntimeState();

std::recursive_mutex &CypherFileSystem_RuntimeMutex();

mount_handle_t CypherFileSystem_AllocateMountHandle( runtime_state_t &state );

fs_error_t CypherFileSystem_InsertMountByPriority(
    runtime_state_t &state,
    const mount_t &mount );

void CypherFileSystem_RemoveMountAtIndex(
    runtime_state_t &state,
    common::u32 index );

fs_error_t CypherFileSystem_ResolveReadableFile(
    const char *virtual_path,
    resolved_file_t &out_file );

bool CypherFileSystem_HasWritePath();

fs_error_t CypherFileSystem_BuildWritePath(
    const char *virtual_path,
    char *out_path,
    common::u32 out_path_size );

}       // namespace cypher::engine::fs

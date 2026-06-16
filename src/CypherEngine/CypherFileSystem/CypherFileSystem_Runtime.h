#ifndef CYPHER_ENGINE_FILESYSTEM_RUNTIME_H
#define CYPHER_ENGINE_FILESYSTEM_RUNTIME_H

#pragma once

#include "CypherEngine/CypherFileSystem/CypherFileSystem.h"
#include "CypherEngine/CypherSystem/CypherSystem_Platform.h"

#include <mutex>

namespace cypher::engine::fs
{

/*
================
Watch filesystem runtime structs

Used for building up the watch structs and entires for communicating with the OS watching.
================
*/
struct watch_snapshot_entry_t {
    bool exists{ false };
    bool is_directory{ false };
    common::u64 size{ 0u };
    std::time_t modified_time{};
    char virtual_path[CYPHER_FILESYSTEM_MAX_PATH_LENGTH]{};
    char physical_path[CYPHER_FILESYSTEM_MAX_PATH_LENGTH]{};
};

struct watch_t {
    watch_handle_t handle{ CYPHER_FILESYSTEM_INVALID_WATCH };
    common::u32 flags{ CYPHER_FILESYSTEM_WATCH_NONE };

    char virtual_path[CYPHER_FILESYSTEM_MAX_PATH_LENGTH]{};
    char physical_path[CYPHER_FILESYSTEM_MAX_PATH_LENGTH]{};

    watch_snapshot_entry_t snapshot[CYPHER_FILESYSTEM_MAX_WATCH_SNAPSHOT_ENTRIES]{};
    common::u32 snapshot_count{ 0u };

#ifdef CYPHER_PLATFORM_WINDOWS
    void *native_handle{ nullptr };
#elif defined( CYPHER_PLATFORM_LINUX )
    int native_fd{ -1 };
    int native_watch{ -1 };
#elif defined( CYPHER_PLATFORM_MACOS )
    int native_fd{ -1 };
#endif

};

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

    /*
     * File watching elements.
     */
    watch_t watches[CYPHER_FILESYSTEM_MAX_WATCHES]{};
    common::u32 watch_count{ 0u };
    watch_handle_t next_watch_handle{ 1u };

    watch_event_t watch_events[CYPHER_FILESYSTEM_MAX_WATCH_EVENTS]{};
    common::u32 watch_event_read_index{ 0u };
    common::u32 watch_event_write_index{ 0u };
    common::u32 watch_event_count{ 0u };
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

#endif // CYPHER_ENGINE_FILESYSTEM_RUNTIME_H

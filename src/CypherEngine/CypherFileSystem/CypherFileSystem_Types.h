#pragma once

#include "CypherEngine/CypherCommon/CypherCommon.h"

#include <ctime>       // std::time_t file timestamps.

namespace cypher::engine::fs
{

constexpr common::u32 CYPHER_FILESYSTEM_MAX_MOUNTS                    = 32u;
constexpr common::u32 CYPHER_FILESYSTEM_MAX_PATH_LENGTH               = 260u;
constexpr common::u32 CYPHER_FILESYSTEM_MAX_VIRTUAL_ROOT_LENGTH       = 64u;

/*
================
Filesystem Types

Virtual mounts map engine paths onto physical storage backends.
================
*/
enum class cypher_filesystem_mount_type_t : common::u8 {
    CYPHER_FILESYSTEM_DIRECTORY    = 0,
    CYPHER_FILESYSTEM_PACKAGE
};

enum cypher_filesystem_mount_flags_t : common::u32 {
    CYPHER_FILESYSTEM_MOUNT_NONE       = 0u,
    CYPHER_FILESYSTEM_MOUNT_READ_ONLY  = 1u << 0u,
    CYPHER_FILESYSTEM_MOUNT_WRITABLE   = 1u << 1u
};

enum class cypher_filesystem_open_mode_t : common::u8 {
    READ_TEXT = 0,
    READ_BINARY,
    WRITE_TEXT,
    WRITE_BINARY,
    APPEND_TEXT,
    APPEND_BINARY
};

enum class cypher_filesystem_file_backend_t : common::u8 {
    INVALID = 0,
    OS_FILE,
    MEMORY_BUFFER,
    MAPPED_FILE
};

enum class cypher_filesystem_seek_origin_t : common::u8 {
    CYPHER_FILESYSTEM_SEEK_START = 0,
    CYPHER_FILESYSTEM_SEEK_CURRENT,
    CYPHER_FILESYSTEM_SEEK_END
};

/*
================
Filesystem Records
================
*/
struct cypher_filesystem_mount_t {
    cypher_filesystem_mount_type_t type{ cypher_filesystem_mount_type_t::CYPHER_FILESYSTEM_DIRECTORY };
    char virtual_root[CYPHER_FILESYSTEM_MAX_VIRTUAL_ROOT_LENGTH]{};
    char physical_root[CYPHER_FILESYSTEM_MAX_PATH_LENGTH]{};
    common::u32 flags{ CYPHER_FILESYSTEM_MOUNT_READ_ONLY };
    common::u32 priority{ 0u };
};

struct cypher_filesystem_file_t {
    cypher_filesystem_file_backend_t backend{ cypher_filesystem_file_backend_t::INVALID };
    void *native_handle{ nullptr };
    common::u64 size{ 0u };
    common::u64 cursor{ 0u };
    bool readable{ false };
    bool writable{ false };
};

struct cypher_filesystem_file_info_t {
    bool exists{ false };
    bool is_directory{ false };
    common::u64 file_size{ 0u };
    std::time_t modified_time{};
    char resolved_path[CYPHER_FILESYSTEM_MAX_PATH_LENGTH]{};
};

}

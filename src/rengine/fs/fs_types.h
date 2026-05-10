#pragma once

#include "rengine/rcommon/com_main.h"

#include <ctime>       // std::time_t file timestamps.

namespace reap::rengine::fs
{

constexpr rcommon::u32 FS_MAX_MOUNTS                    = 32u;
constexpr rcommon::u32 FS_MAX_PATH_LENGTH               = 260u;
constexpr rcommon::u32 FS_MAX_VIRTUAL_ROOT_LENGTH       = 64u;

/*
================
Filesystem Types

Virtual mounts map engine paths onto physical storage backends.
================
*/
enum class fs_mount_type_t : rcommon::u8 {
    FS_DIRECTORY    = 0,
    FS_PACKAGE
};

enum fs_mount_flags_t : rcommon::u32 {
    FS_MOUNT_NONE       = 0u,
    FS_MOUNT_READ_ONLY  = 1u << 0u,
    FS_MOUNT_WRITABLE   = 1u << 1u

};

enum class fs_open_mode_t : rcommon::u8 {
    READ_TEXT = 0,
    READ_BINARY,
    WRITE_TEXT,
    WRITE_BINARY,
    APPEND_TEXT,
    APPEND_BINARY
};

enum class fs_file_backend_t : rcommon::u8 {
    INVALID = 0,
    OS_FILE,
    MEMORY_BUFFER,
    MAPPED_FILE
};

enum class fs_seek_origin_t : rcommon::u8 {
    FS_SEEK_START = 0,
    FS_SEEK_CURRENT,
    FS_SEEK_END
};

/*
================
Filesystem Records
================
*/
struct fs_mount_t {
    fs_mount_type_t type{ fs_mount_type_t::FS_DIRECTORY };
    char virtual_root[FS_MAX_VIRTUAL_ROOT_LENGTH]{};
    char physical_root[FS_MAX_PATH_LENGTH]{};
    rcommon::u32 flags{ FS_MOUNT_READ_ONLY };
    rcommon::u32 priority{ 0u };
};

struct fs_file_t {
    fs_file_backend_t backend{ fs_file_backend_t::INVALID };
    void *native_handle{ nullptr };
    rcommon::u64 size{ 0u };
    rcommon::u64 cursor{ 0u };
    bool readable{ false };
    bool writable{ false };
};

struct fs_file_info_t {
    bool exists{ false };
    bool is_directory{ false };
    rcommon::u64 file_size{ 0u };
    std::time_t modified_time{};
    char resolved_path[FS_MAX_PATH_LENGTH]{};
};

}

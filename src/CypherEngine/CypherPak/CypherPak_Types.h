#ifndef CYPHER_ENGINE_PAK_TYPES_H
#define CYPHER_ENGINE_PAK_TYPES_H

#pragma once

#include "CypherEngine/CypherPak/CypherPak_Error.h"
#include "CypherEngine/CypherPak/CypherPak_Format.h"

namespace cypher::engine::pak
{

inline constexpr common::u32 CYPHER_PAK_INVALID_HANDLE = 0u;
inline constexpr common::u32 CYPHER_PAK_INVALID_FILE_INDEX = 0xFFFFFFFFu;
inline constexpr common::u32 CYPHER_PAK_MAX_ARCHIVE_PATH_LENGTH = CYPHER_PAK_MAX_PATH_LENGTH;

using pak_handle_t = common::u32;
using pak_file_index_t = common::u32;

enum pak_open_flags_t : common::u32 {
    CYPHER_PAK_OPEN_NONE                = 0u,
    CYPHER_PAK_OPEN_VERIFY_HEADER       = 1u << 0u,
    CYPHER_PAK_OPEN_VERIFY_INDEX        = 1u << 1u,
    CYPHER_PAK_OPEN_VERIFY_FILE_HASHES  = 1u << 2u,
    CYPHER_PAK_OPEN_MEMORY_MAP          = 1u << 3u
};

enum pak_verify_flags_t : common::u32 {
    CYPHER_PAK_VERIFY_NONE              = 0u,
    CYPHER_PAK_VERIFY_HEADER            = 1u << 0u,
    CYPHER_PAK_VERIFY_INDEX             = 1u << 1u,
    CYPHER_PAK_VERIFY_FILE_HASHES       = 1u << 2u,
    CYPHER_PAK_VERIFY_ARCHIVE_HASH      = 1u << 3u,
    CYPHER_PAK_VERIFY_FULL              = CYPHER_PAK_VERIFY_HEADER |
                                          CYPHER_PAK_VERIFY_INDEX |
                                          CYPHER_PAK_VERIFY_FILE_HASHES |
                                          CYPHER_PAK_VERIFY_ARCHIVE_HASH
};

struct pak_file_info_t {
    char virtual_path[CYPHER_PAK_MAX_PATH_LENGTH]{};
    pak_file_index_t index{ CYPHER_PAK_INVALID_FILE_INDEX };
    common::u64 data_offset{ 0u };
    common::u64 stored_size{ 0u };
    common::u64 unpacked_size{ 0u };
    common::u64 modified_time_utc{ 0u };
    common::u64 content_hash{ 0u };
    common::u32 path_hash{ 0u };
    pak_compression_t compression{ pak_compression_t::NONE };
    common::u32 flags{ CYPHER_PAK_ENTRY_NONE };
};

struct pak_stats_t {
    common::u64 file_count{ 0u };
    common::u64 archive_size{ 0u };
    common::u64 compressed_file_count{ 0u };
    common::u64 stored_data_size{ 0u };
    common::u64 unpacked_data_size{ 0u };
    common::u64 read_count{ 0u };
    common::u64 bytes_read{ 0u };
};

struct pak_reader_t {
    pak_handle_t handle{ CYPHER_PAK_INVALID_HANDLE };
    void *native_file{ nullptr };
    char archive_path[CYPHER_PAK_MAX_ARCHIVE_PATH_LENGTH]{};
    pak_header_t header{};
    pak_disk_file_entry_t *entries{ nullptr };
    char *string_table{ nullptr };
    common::u64 string_table_size{ 0u };
    common::u32 file_count{ 0u };
    common::u32 flags{ CYPHER_PAK_OPEN_NONE };
    pak_stats_t stats{};
    bool open{ false };
};

struct pak_writer_t {
    pak_handle_t handle{ CYPHER_PAK_INVALID_HANDLE };
    void *native_file{ nullptr };
    void *builder_state{ nullptr };
    char archive_path[CYPHER_PAK_MAX_ARCHIVE_PATH_LENGTH]{};
    common::u32 flags{ 0u };
    common::u32 data_alignment{ CYPHER_PAK_DATA_ALIGNMENT };
    pak_compression_t default_compression{ pak_compression_t::NONE };
    common::u32 file_count{ 0u };
    bool open{ false };
    bool finalized{ false };
};

struct pak_source_file_t {
    const char *virtual_path{ nullptr };
    const char *physical_path{ nullptr };
    pak_compression_t compression{ pak_compression_t::NONE };
    common::u32 flags{ CYPHER_PAK_ENTRY_NONE };
};

}       // namespace cypher::engine::pak

#endif // CYPHER_ENGINE_PAK_TYPES_H

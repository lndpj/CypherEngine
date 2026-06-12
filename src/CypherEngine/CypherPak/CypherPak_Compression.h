#pragma once

#include "CypherEngine/CypherPak/CypherPak_Types.h"

namespace cypher::engine::pak
{

enum pak_compression_flags_t : common::u32 {
    CYPHER_PAK_COMPRESS_NONE            = 0u,
    CYPHER_PAK_COMPRESS_FAST            = 1u << 0u,
    CYPHER_PAK_COMPRESS_BEST            = 1u << 1u,
    CYPHER_PAK_COMPRESS_DETERMINISTIC   = 1u << 2u
};

struct pak_compression_config_t {
    pak_compression_t method{ pak_compression_t::NONE };
    common::u32 flags{ CYPHER_PAK_COMPRESS_NONE };
    common::u32 level{ 0u };
    common::u32 chunk_size{ 0u };
};

const char *CypherPak_CompressionName( pak_compression_t method );

bool CypherPak_CompressionSupported( pak_compression_t method );

pak_error_t CypherPak_CompressBound(
    pak_compression_t method,
    common::u64 input_size,
    common::u64 &out_max_output_size );

pak_error_t CypherPak_Compress(
    const pak_compression_config_t &config,
    const void *input,
    common::u64 input_size,
    void *output,
    common::u64 output_size,
    common::u64 &out_bytes_written );

pak_error_t CypherPak_Decompress(
    pak_compression_t method,
    const void *input,
    common::u64 input_size,
    void *output,
    common::u64 output_size,
    common::u64 &out_bytes_written );

}       // namespace cypher::engine::pak

#ifndef CYPHER_COMMON_TIER1_COMPRESSION_H
#define CYPHER_COMMON_TIER1_COMPRESSION_H
#pragma once

/*
================
CypherCommon Compression

Common compression API declarations.
================
*/

#include "CypherCommon_Tier0.h"

namespace cypher::common
{

enum class compression_codec_t : u32 {
    None = 0u,
    LZ,
    LZ4,
    Zstd
};

usize Compression_CompressBound( compression_codec_t codec, usize cbInput );
bool_t Compression_Compress( compression_codec_t codec, const void *pInput, usize cbInput, void *pOutput, usize cbOutput, usize *pOutWritten );
bool_t Compression_Decompress( compression_codec_t codec, const void *pInput, usize cbInput, void *pOutput, usize cbOutput, usize *pOutWritten );

} // namespace cypher::common

#endif // CYPHER_COMMON_TIER1_COMPRESSION_H

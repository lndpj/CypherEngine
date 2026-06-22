#ifndef CYPHER_COMMON_TIER1_COMPRESSIONZSTD_H
#define CYPHER_COMMON_TIER1_COMPRESSIONZSTD_H
#pragma once

/*
================
CypherCommon Compression Zstd

Zstandard compression integration declarations.
================
*/

#include "CypherCommon_Tier0.h"

namespace cypher::common
{

usize CompressionZstd_CompressBound( usize cbInput );
bool_t CompressionZstd_Compress( const void *pInput, usize cbInput, void *pOutput, usize cbOutput, usize *pOutWritten );
bool_t CompressionZstd_Decompress( const void *pInput, usize cbInput, void *pOutput, usize cbOutput, usize *pOutWritten );

} // namespace cypher::common

#endif // CYPHER_COMMON_TIER1_COMPRESSIONZSTD_H

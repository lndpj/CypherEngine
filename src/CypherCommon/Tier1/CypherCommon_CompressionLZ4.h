#ifndef CYPHER_COMMON_TIER1_COMPRESSIONLZ4_H
#define CYPHER_COMMON_TIER1_COMPRESSIONLZ4_H
#pragma once

/*
================
CypherCommon Compression LZ4

LZ4 compression integration declarations.
================
*/

#include "CypherCommon_Tier0.h"

namespace cypher::common
{

usize CompressionLZ4_CompressBound( usize cbInput );
bool_t CompressionLZ4_Compress( const void *pInput, usize cbInput, void *pOutput, usize cbOutput, usize *pOutWritten );
bool_t CompressionLZ4_Decompress( const void *pInput, usize cbInput, void *pOutput, usize cbOutput, usize *pOutWritten );

} // namespace cypher::common

#endif // CYPHER_COMMON_TIER1_COMPRESSIONLZ4_H

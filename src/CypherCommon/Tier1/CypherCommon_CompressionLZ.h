#ifndef CYPHER_COMMON_TIER1_COMPRESSIONLZ_H
#define CYPHER_COMMON_TIER1_COMPRESSIONLZ_H
#pragma once

/*
================
CypherCommon Compression LZ

Simple LZ compression declarations.
================
*/

#include "CypherCommon_Tier0.h"

namespace cypher::common
{

usize CompressionLZ_CompressBound( usize cbInput );
bool_t CompressionLZ_Compress( const void *pInput, usize cbInput, void *pOutput, usize cbOutput, usize *pOutWritten );
bool_t CompressionLZ_Decompress( const void *pInput, usize cbInput, void *pOutput, usize cbOutput, usize *pOutWritten );

} // namespace cypher::common

#endif // CYPHER_COMMON_TIER1_COMPRESSIONLZ_H

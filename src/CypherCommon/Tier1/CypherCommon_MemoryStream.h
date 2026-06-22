#ifndef CYPHER_COMMON_TIER1_MEMORYSTREAM_H
#define CYPHER_COMMON_TIER1_MEMORYSTREAM_H
#pragma once

/*
================
CypherCommon Memory Stream

Stream backed by memory declarations.
================
*/

#include "CypherCommon_Tier0.h"

namespace cypher::common
{

struct memory_stream_t {
    byte *pData;
    usize cbSize;
    usize cbCapacity;
    usize offset;
};

void MemoryStream_Init( memory_stream_t *pStream, void *pData, usize cbCapacity );
usize MemoryStream_Read( memory_stream_t *pStream, void *pDest, usize cbRead );
usize MemoryStream_Write( memory_stream_t *pStream, const void *pSrc, usize cbWrite );
bool_t MemoryStream_Seek( memory_stream_t *pStream, usize offset );

} // namespace cypher::common

#endif // CYPHER_COMMON_TIER1_MEMORYSTREAM_H

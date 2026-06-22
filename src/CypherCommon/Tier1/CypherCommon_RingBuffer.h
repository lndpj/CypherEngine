#ifndef CYPHER_COMMON_TIER1_RINGBUFFER_H
#define CYPHER_COMMON_TIER1_RINGBUFFER_H
#pragma once

/*
================
CypherCommon Ring Buffer

Fixed-size circular buffer declarations.
================
*/

#include "CypherCommon_Tier0.h"

namespace cypher::common
{

struct ring_buffer_t;

bool_t RingBuffer_Init( ring_buffer_t *pBuffer, void *pMemory, usize cbMemory );
usize RingBuffer_Write( ring_buffer_t *pBuffer, const void *pData, usize cbData );
usize RingBuffer_Read( ring_buffer_t *pBuffer, void *pData, usize cbData );

} // namespace cypher::common

#endif // CYPHER_COMMON_TIER1_RINGBUFFER_H

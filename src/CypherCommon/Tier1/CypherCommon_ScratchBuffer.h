#ifndef CYPHER_COMMON_TIER1_SCRATCHBUFFER_H
#define CYPHER_COMMON_TIER1_SCRATCHBUFFER_H
#pragma once

/*
================
CypherCommon Scratch Buffer

Temporary byte buffer declarations.
================
*/

#include "CypherCommon_Tier0.h"

namespace cypher::common
{

struct scratch_buffer_t {
    byte *pData;
    usize cbCapacity;
    usize cbUsed;
};

void ScratchBuffer_Init( scratch_buffer_t *pBuffer, void *pMemory, usize cbCapacity );
void *ScratchBuffer_Alloc( scratch_buffer_t *pBuffer, usize cbSize, usize alignment );
void ScratchBuffer_Reset( scratch_buffer_t *pBuffer );

} // namespace cypher::common

#endif // CYPHER_COMMON_TIER1_SCRATCHBUFFER_H

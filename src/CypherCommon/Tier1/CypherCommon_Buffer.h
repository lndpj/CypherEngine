#ifndef CYPHER_COMMON_TIER1_BUFFER_H
#define CYPHER_COMMON_TIER1_BUFFER_H
#pragma once

/*
================
CypherCommon Buffer

Mutable byte buffer declarations.
================
*/

#include "CypherCommon_Tier0.h"

namespace cypher::common
{

struct buffer_t {
    byte *pData;
    usize cbSize;
    usize cbCapacity;
};

void Buffer_Init( buffer_t *pBuffer, void *pMemory, usize cbCapacity );
bool_t Buffer_Reserve( buffer_t *pBuffer, usize cbCapacity );
bool_t Buffer_Append( buffer_t *pBuffer, const void *pData, usize cbData );
void Buffer_Clear( buffer_t *pBuffer );

} // namespace cypher::common

#endif // CYPHER_COMMON_TIER1_BUFFER_H

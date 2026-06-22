#ifndef CYPHER_COMMON_TIER1_BITBUFFER_H
#define CYPHER_COMMON_TIER1_BITBUFFER_H
#pragma once

/*
================
CypherCommon Bit Buffer

Combined bit reader/writer declarations.
================
*/

#include "CypherCommon_BitReader.h"
#include "CypherCommon_BitWriter.h"

namespace cypher::common
{

struct bit_buffer_t {
    bit_reader_t reader;
    bit_writer_t writer;
};

void BitBuffer_Init( bit_buffer_t *pBuffer, void *pData, usize cbCapacity );

} // namespace cypher::common

#endif // CYPHER_COMMON_TIER1_BITBUFFER_H

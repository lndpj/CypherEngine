#ifndef CYPHER_COMMON_TIER1_BYTEREADER_H
#define CYPHER_COMMON_TIER1_BYTEREADER_H
#pragma once

/*
================
CypherCommon Byte Reader

Sequential byte reader declarations.
================
*/

#include "CypherCommon_Tier0.h"

namespace cypher::common
{

struct byte_reader_t {
    const byte *pData;
    usize cbSize;
    usize offset;
};

void ByteReader_Init( byte_reader_t *pReader, const void *pData, usize cbSize );
bool_t ByteReader_Read( byte_reader_t *pReader, void *pDest, usize cbData );
bool_t ByteReader_Skip( byte_reader_t *pReader, usize cbData );
usize ByteReader_Remaining( const byte_reader_t *pReader );

} // namespace cypher::common

#endif // CYPHER_COMMON_TIER1_BYTEREADER_H

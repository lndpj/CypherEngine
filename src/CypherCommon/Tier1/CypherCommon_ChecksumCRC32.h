#ifndef CYPHER_COMMON_TIER1_CHECKSUMCRC32_H
#define CYPHER_COMMON_TIER1_CHECKSUMCRC32_H
#pragma once

/*
================
CypherCommon CRC32

CRC32 checksum declarations.
================
*/

#include "CypherCommon_Tier0.h"

namespace cypher::common
{

crc32_t ChecksumCRC32_Data( const void *pData, usize cbData );
crc32_t ChecksumCRC32_Update( crc32_t crc, const void *pData, usize cbData );

} // namespace cypher::common

#endif // CYPHER_COMMON_TIER1_CHECKSUMCRC32_H

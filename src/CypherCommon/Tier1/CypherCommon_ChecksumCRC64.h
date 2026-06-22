#ifndef CYPHER_COMMON_TIER1_CHECKSUMCRC64_H
#define CYPHER_COMMON_TIER1_CHECKSUMCRC64_H
#pragma once

/*
================
CypherCommon CRC64

CRC64 checksum declarations.
================
*/

#include "CypherCommon_Tier0.h"

namespace cypher::common
{

using crc64_t = u64;

crc64_t ChecksumCRC64_Data( const void *pData, usize cbData );
crc64_t ChecksumCRC64_Update( crc64_t crc, const void *pData, usize cbData );

} // namespace cypher::common

#endif // CYPHER_COMMON_TIER1_CHECKSUMCRC64_H

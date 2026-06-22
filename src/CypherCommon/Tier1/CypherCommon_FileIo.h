#ifndef CYPHER_COMMON_TIER1_FILEIO_H
#define CYPHER_COMMON_TIER1_FILEIO_H
#pragma once

/*
================
CypherCommon File IO

Small raw file utility declarations. VFS policy stays in CypherFileSystem.
================
*/

#include "CypherCommon_Tier0.h"

namespace cypher::common
{

bool_t FileIo_ReadAllBytes( const char *pPath, void *pDest, usize cbDest, usize *pOutBytesRead );
bool_t FileIo_WriteAllBytes( const char *pPath, const void *pData, usize cbData );
bool_t FileIo_Exists( const char *pPath );

} // namespace cypher::common

#endif // CYPHER_COMMON_TIER1_FILEIO_H

#ifndef CYPHER_COMMON_TIER1_BLOB_H
#define CYPHER_COMMON_TIER1_BLOB_H
#pragma once

/*
================
CypherCommon Blob

Owned binary memory block declarations used by VFS reads, pak entries, tools,
asset importers and serialized data.
================
*/

#include "CypherCommon_Tier0.h"

namespace cypher::common
{

struct blob_t {
    void *pData;
    usize cbSize;
    usize cbCapacity;
};

bool_t Blob_Init( blob_t *pBlob, usize cbInitialCapacity );
void Blob_Free( blob_t *pBlob );
bool_t Blob_Resize( blob_t *pBlob, usize cbSize );
bool_t Blob_Reserve( blob_t *pBlob, usize cbCapacity );
bool_t Blob_Assign( blob_t *pBlob, const void *pData, usize cbData );
void Blob_Clear( blob_t *pBlob );

} // namespace cypher::common

#endif // CYPHER_COMMON_TIER1_BLOB_H

#ifndef CYPHER_COMMON_TIER1_BINARYBLOCK_H
#define CYPHER_COMMON_TIER1_BINARYBLOCK_H
#pragma once

/*
================
CypherCommon Binary Block

Owned or referenced binary blob declarations.
================
*/

#include "CypherCommon_Tier0.h"

namespace cypher::common
{

struct binary_block_t {
    void *pData;
    usize cbSize;
};

void BinaryBlock_Clear( binary_block_t *pBlock );
bool_t BinaryBlock_IsEmpty( const binary_block_t *pBlock );

} // namespace cypher::common

#endif // CYPHER_COMMON_TIER1_BINARYBLOCK_H

#ifndef CYPHER_COMMON_TIER1_KEYVALUEPACK_H
#define CYPHER_COMMON_TIER1_KEYVALUEPACK_H
#pragma once

/*
================
CypherCommon KeyValue Pack

Binary packed key/value declarations.
================
*/

#include "CypherCommon_KeyValue.h"

namespace cypher::common
{

usize KeyValuePack_Size( const key_value_t *pRoot );
bool_t KeyValuePack_Write( const key_value_t *pRoot, void *pDest, usize cbDest );
bool_t KeyValuePack_Read( const void *pData, usize cbData, key_value_t **ppOutRoot );

} // namespace cypher::common

#endif // CYPHER_COMMON_TIER1_KEYVALUEPACK_H

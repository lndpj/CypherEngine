#ifndef CYPHER_COMMON_TIER1_KEYVALUE_H
#define CYPHER_COMMON_TIER1_KEYVALUE_H
#pragma once

/*
================
CypherCommon KeyValue

Hierarchical key/value data declarations.
================
*/

#include "CypherCommon_Tier0.h"

namespace cypher::common
{

enum class key_value_type_t : u32 {
    Null = 0u,
    String,
    Integer,
    Float,
    Boolean,
    Object,
    Array
};

struct key_value_t;

key_value_t *KeyValue_Find( key_value_t *pRoot, const char *pName );
const key_value_t *KeyValue_Find( const key_value_t *pRoot, const char *pName );
key_value_type_t KeyValue_GetType( const key_value_t *pValue );

} // namespace cypher::common

#endif // CYPHER_COMMON_TIER1_KEYVALUE_H

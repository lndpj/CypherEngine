#ifndef CYPHER_COMMON_TIER1_KEYVALUEJSON_H
#define CYPHER_COMMON_TIER1_KEYVALUEJSON_H
#pragma once

/*
================
CypherCommon KeyValue JSON

JSON conversion declarations for key/value data.
================
*/

#include "CypherCommon_KeyValue.h"

namespace cypher::common
{

bool_t KeyValueJson_Parse( const char *pText, key_value_t **ppOutRoot );
usize KeyValueJson_Write( const key_value_t *pRoot, char *pDest, usize cchDest );

} // namespace cypher::common

#endif // CYPHER_COMMON_TIER1_KEYVALUEJSON_H

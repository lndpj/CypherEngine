#ifndef CYPHER_COMMON_TIER1_KEYVALUEPARSER_H
#define CYPHER_COMMON_TIER1_KEYVALUEPARSER_H
#pragma once

/*
================
CypherCommon KeyValue Parser

Key/value parser declarations.
================
*/

#include "CypherCommon_KeyValue.h"

namespace cypher::common
{

struct key_value_parse_result_t {
    key_value_t *pRoot;
    u32 error_line;
    const char *pError;
};

bool_t KeyValue_ParseText( const char *pText, key_value_parse_result_t *pOutResult );
bool_t KeyValue_ParseFileData( const void *pData, usize cbData, key_value_parse_result_t *pOutResult );

} // namespace cypher::common

#endif // CYPHER_COMMON_TIER1_KEYVALUEPARSER_H

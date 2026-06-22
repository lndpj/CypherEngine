#ifndef CYPHER_COMMON_TIER1_KEYVALUEWRITER_H
#define CYPHER_COMMON_TIER1_KEYVALUEWRITER_H
#pragma once

/*
================
CypherCommon KeyValue Writer

Key/value text writer declarations.
================
*/

#include "CypherCommon_KeyValue.h"

namespace cypher::common
{

usize KeyValue_WriteText( const key_value_t *pRoot, char *pDest, usize cchDest );
usize KeyValue_WriteCompactText( const key_value_t *pRoot, char *pDest, usize cchDest );

} // namespace cypher::common

#endif // CYPHER_COMMON_TIER1_KEYVALUEWRITER_H

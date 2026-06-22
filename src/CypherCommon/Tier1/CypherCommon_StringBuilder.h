#ifndef CYPHER_COMMON_TIER1_STRINGBUILDER_H
#define CYPHER_COMMON_TIER1_STRINGBUILDER_H
#pragma once

/*
================
CypherCommon String Builder

Caller-buffer string construction declarations.
================
*/

#include "CypherCommon_Tier0.h"

namespace cypher::common
{

struct string_builder_t {
    char *pBuffer;
    usize cchCapacity;
    usize cchLength;
};

void StringBuilder_Init( string_builder_t *pBuilder, char *pBuffer, usize cchCapacity );
usize StringBuilder_Append( string_builder_t *pBuilder, const char *pString );
usize StringBuilder_AppendChar( string_builder_t *pBuilder, char ch );
usize StringBuilder_Format( string_builder_t *pBuilder, const char *pFormat, ... );

} // namespace cypher::common

#endif // CYPHER_COMMON_TIER1_STRINGBUILDER_H

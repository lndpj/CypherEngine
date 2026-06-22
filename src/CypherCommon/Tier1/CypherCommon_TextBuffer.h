#ifndef CYPHER_COMMON_TIER1_TEXTBUFFER_H
#define CYPHER_COMMON_TIER1_TEXTBUFFER_H
#pragma once

/*
================
CypherCommon Text Buffer

Owned text buffer declarations for shaders, configs, scripts, material files,
logs and editor text tools.
================
*/

#include "CypherCommon_Tier0.h"

namespace cypher::common
{

struct text_buffer_t {
    char *pData;
    usize cchLength;
    usize cchCapacity;
};

bool_t TextBuffer_Init( text_buffer_t *pBuffer, usize cchInitialCapacity );
void TextBuffer_Free( text_buffer_t *pBuffer );
bool_t TextBuffer_Assign( text_buffer_t *pBuffer, const char *pText, usize cchText );
bool_t TextBuffer_Append( text_buffer_t *pBuffer, const char *pText );
bool_t TextBuffer_AppendN( text_buffer_t *pBuffer, const char *pText, usize cchText );
void TextBuffer_Clear( text_buffer_t *pBuffer );

} // namespace cypher::common

#endif // CYPHER_COMMON_TIER1_TEXTBUFFER_H

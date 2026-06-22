#ifndef CYPHER_COMMON_TIER0_WIDECHAR_H
#define CYPHER_COMMON_TIER0_WIDECHAR_H
#pragma once

/*
================
CypherCommon Wide Char

Wide character declarations for platform boundaries.
================
*/

#include "CypherCommon_BaseTypes.h"

namespace cypher::common
{

using wchar_engine_t = wchar_t;

usize WChar_Length( const wchar_engine_t *pString );

i32 WChar_Compare( const wchar_engine_t *pStringA, const wchar_engine_t *pStringB );

usize WChar_Copy( wchar_engine_t *pDest, const wchar_engine_t *pSrc, usize cchDest );

} // namespace cypher::common

#endif // CYPHER_COMMON_TIER0_WIDECHAR_H

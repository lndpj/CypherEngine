#ifndef CYPHER_COMMON_TIER1_STRINGCONVERT_H
#define CYPHER_COMMON_TIER1_STRINGCONVERT_H
#pragma once

/*
================
CypherCommon String Convert

Numeric, boolean, hexadecimal and binary text conversion helpers.
================
*/

#include "CypherCommon_Tier0.h"

namespace cypher::common
{

/*
================
Classic Numeric Conversion
================
*/
// Converts pString to a signed 32-bit integer.
i32 Cy_atoi( const char *pString );

// Converts pString to a signed 64-bit integer.
i64 Cy_atoi64( const char *pString );

// Converts pString to an unsigned 64-bit integer.
u64 Cy_atoui64( const char *pString );

// Converts pString to a 32-bit floating-point value.
f32 Cy_atof( const char *pString );

/*
================
Checked Numeric Conversion
================
*/
// Parses a signed 32-bit integer and reports success.
bool_t Cy_strtoi32( const char *pString, i32 *pOutValue );

// Parses a signed 64-bit integer and reports success.
bool_t Cy_strtoi64( const char *pString, i64 *pOutValue );

// Parses an unsigned 32-bit integer and reports success.
bool_t Cy_strtou32( const char *pString, u32 *pOutValue );

// Parses an unsigned 64-bit integer and reports success.
bool_t Cy_strtou64( const char *pString, u64 *pOutValue );

// Parses a 32-bit floating-point value and reports success.
bool_t Cy_strtof32( const char *pString, f32 *pOutValue );

// Parses a boolean value and reports success.
bool_t Cy_strtobool( const char *pString, bool_t *pOutValue );

/*
================
Hex / Binary Text
================
*/
// Returns true when pString contains only valid hexadecimal digits.
bool_t Cy_isvalidhex( const char *pString );

// Writes binary data as lowercase hexadecimal text.
usize Cy_binarytohex( const void *pData, usize cbData, char *pDest, usize cchDest );

// Converts hexadecimal text into binary bytes.
usize Cy_hextobinary( const char *pString, void *pDest, usize cbDest );

} // namespace cypher::common

#endif // CYPHER_COMMON_TIER1_STRINGCONVERT_H

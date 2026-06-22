#ifndef CYPHER_COMMON_TIER1_STRINGURL_H
#define CYPHER_COMMON_TIER1_STRINGURL_H
#pragma once

/*
================
CypherCommon String URL

URL text helpers for tools, server browser, launch links and future HTTP.
================
*/

#include "CypherCommon_Tier0.h"

namespace cypher::common
{

// Returns true when ch is valid in a URL.
bool_t Cy_IsValidUrlChar( char ch );

// Returns true when ch is valid in a domain name.
bool_t Cy_IsValidDomainNameChar( char ch );

// Encodes pString as URL-safe text.
usize Cy_UrlEncode( const char *pString, char *pDest, usize cchDest );

// Encodes pData as URL-safe text without assuming null-terminated input.
usize Cy_UrlEncodeRaw( const void *pData, usize cbData, char *pDest, usize cchDest );

// Decodes URL-escaped text.
usize Cy_UrlDecode( const char *pString, char *pDest, usize cchDest );

// Decodes URL-escaped text into raw bytes.
usize Cy_UrlDecodeRaw( const char *pString, void *pDest, usize cbDest );

// Extracts the domain portion from pUrl.
usize Cy_ExtractDomainFromUrl( const char *pUrl, char *pDest, usize cchDest );

// Returns true when pUrl contains pDomain as its domain.
bool_t Cy_UrlContainsDomain( const char *pUrl, const char *pDomain );

} // namespace cypher::common

#endif // CYPHER_COMMON_TIER1_STRINGURL_H

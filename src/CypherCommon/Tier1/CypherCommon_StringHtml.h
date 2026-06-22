#ifndef CYPHER_COMMON_TIER1_STRINGHTML_H
#define CYPHER_COMMON_TIER1_STRINGHTML_H
#pragma once

/*
================
CypherCommon String HTML

Small HTML entity helpers for tools, logs, launcher/server browser UI and docs.
================
*/

#include "CypherCommon_Tier0.h"

namespace cypher::common
{

// Encodes basic HTML entities such as <, >, &, quotes.
usize Cy_HtmlEntityEncode( const char *pString, char *pDest, usize cchDest );

// Decodes basic HTML entities into text.
usize Cy_HtmlEntityDecode( const char *pString, char *pDest, usize cchDest );

// Strips HTML tags while preserving visible text.
usize Cy_StripHtml( const char *pString, char *pDest, usize cchDest );

// Strips HTML tags while preserving supported formatting markers.
usize Cy_StripAndPreserveHtml( const char *pString, char *pDest, usize cchDest );

} // namespace cypher::common

#endif // CYPHER_COMMON_TIER1_STRINGHTML_H

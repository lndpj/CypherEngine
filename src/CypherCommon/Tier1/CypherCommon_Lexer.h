#ifndef CYPHER_COMMON_TIER1_LEXER_H
#define CYPHER_COMMON_TIER1_LEXER_H
#pragma once

/*
================
CypherCommon Lexer

Configurable lexer declarations for tools and data formats.
================
*/

#include "CypherCommon_Tier0.h"
#include "CypherCommon_TokenReader.h"

namespace cypher::common
{

struct lexer_rules_t {
    bool_t allow_comments;
    bool_t allow_quoted_strings;
    bool_t case_sensitive;
};

struct lexer_t;

void Lexer_Init( lexer_t *pLexer, const char *pText, const lexer_rules_t *pRules );
bool_t Lexer_ReadToken( lexer_t *pLexer, token_t *pOutToken );

} // namespace cypher::common

#endif // CYPHER_COMMON_TIER1_LEXER_H

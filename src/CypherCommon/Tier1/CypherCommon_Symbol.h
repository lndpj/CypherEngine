#ifndef CYPHER_COMMON_TIER1_SYMBOL_H
#define CYPHER_COMMON_TIER1_SYMBOL_H
#pragma once

/*
================
CypherCommon Symbol

String symbol table declarations.
================
*/

#include "CypherCommon_Tier0.h"

namespace cypher::common
{

using symbol_id_t = u32;

constexpr symbol_id_t CY_SYMBOL_INVALID = CY_U32_MAX;

struct symbol_table_t;

symbol_id_t SymbolTable_Add( symbol_table_t *pTable, const char *pString );
symbol_id_t SymbolTable_Find( const symbol_table_t *pTable, const char *pString );
const char *SymbolTable_GetString( const symbol_table_t *pTable, symbol_id_t symbol );

} // namespace cypher::common

#endif // CYPHER_COMMON_TIER1_SYMBOL_H

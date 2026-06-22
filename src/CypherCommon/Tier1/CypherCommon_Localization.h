#ifndef CYPHER_COMMON_TIER1_LOCALIZATION_H
#define CYPHER_COMMON_TIER1_LOCALIZATION_H
#pragma once

/*
================
CypherCommon Localization

Text lookup declarations for game UI, tools and editor strings. The actual
localization database can stay outside Common.
================
*/

#include "CypherCommon_Tier0.h"

namespace cypher::common
{

using loc_string_id_t = hash32_t;

struct localization_entry_t {
    loc_string_id_t id;
    const char *pKey;
    const char *pText;
};

struct localization_table_t;

bool_t Localization_Init( localization_table_t *pTable, u32 maxEntries );
void Localization_Shutdown( localization_table_t *pTable );
bool_t Localization_AddString( localization_table_t *pTable, const char *pKey, const char *pText );
const char *Localization_FindText( const localization_table_t *pTable, const char *pKey );
const char *Localization_FindTextById( const localization_table_t *pTable, loc_string_id_t id );

} // namespace cypher::common

#endif // CYPHER_COMMON_TIER1_LOCALIZATION_H

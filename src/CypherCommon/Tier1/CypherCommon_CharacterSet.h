#ifndef CYPHER_COMMON_TIER1_CHARACTERSET_H
#define CYPHER_COMMON_TIER1_CHARACTERSET_H
#pragma once

/*
================
CypherCommon Character Set

ASCII character set declarations used by tokenizers and parsers.
================
*/

#include "CypherCommon_Tier0.h"

namespace cypher::common
{

struct character_set_t {
    u8 bits[32];
};

void CharacterSet_Clear( character_set_t *pSet );
void CharacterSet_Add( character_set_t *pSet, char ch );
void CharacterSet_AddRange( character_set_t *pSet, char chFirst, char chLast );
bool_t CharacterSet_Contains( const character_set_t *pSet, char ch );

} // namespace cypher::common

#endif // CYPHER_COMMON_TIER1_CHARACTERSET_H

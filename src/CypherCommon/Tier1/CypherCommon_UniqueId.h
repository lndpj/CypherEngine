#ifndef CYPHER_COMMON_TIER1_UNIQUEID_H
#define CYPHER_COMMON_TIER1_UNIQUEID_H
#pragma once

/*
================
CypherCommon Unique ID

Unique identifier declarations.
================
*/

#include "CypherCommon_Tier0.h"

namespace cypher::common
{

struct unique_id_t {
    u64 high;
    u64 low;
};

unique_id_t UniqueId_Create();
unique_id_t UniqueId_FromString( const char *pString );
usize UniqueId_ToString( unique_id_t id, char *pDest, usize cchDest );
bool_t UniqueId_Equals( unique_id_t a, unique_id_t b );

} // namespace cypher::common

#endif // CYPHER_COMMON_TIER1_UNIQUEID_H

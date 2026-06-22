#ifndef CYPHER_COMMON_TIER1_PASSWORDHASH_H
#define CYPHER_COMMON_TIER1_PASSWORDHASH_H
#pragma once

/*
================
CypherCommon Password Hash

Password hash declaration surface for future security integrations.
================
*/

#include "CypherCommon_Tier0.h"

namespace cypher::common
{

bool_t PasswordHash_Create( const char *pPassword, char *pDest, usize cchDest );
bool_t PasswordHash_Verify( const char *pPassword, const char *pHash );

} // namespace cypher::common

#endif // CYPHER_COMMON_TIER1_PASSWORDHASH_H

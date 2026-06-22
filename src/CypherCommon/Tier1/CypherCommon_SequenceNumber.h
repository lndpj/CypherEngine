#ifndef CYPHER_COMMON_TIER1_SEQUENCENUMBER_H
#define CYPHER_COMMON_TIER1_SEQUENCENUMBER_H
#pragma once

/*
================
CypherCommon Sequence Number

Wrapping sequence number declarations for networking.
================
*/

#include "CypherCommon_Tier0.h"

namespace cypher::common
{

bool_t SequenceNumber_IsNewer16( u16 a, u16 b );
bool_t SequenceNumber_IsNewer32( u32 a, u32 b );
i32 SequenceNumber_Diff16( u16 a, u16 b );
i32 SequenceNumber_Diff32( u32 a, u32 b );

} // namespace cypher::common

#endif // CYPHER_COMMON_TIER1_SEQUENCENUMBER_H

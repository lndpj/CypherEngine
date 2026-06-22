#ifndef CYPHER_COMMON_TIER0_TLS_H
#define CYPHER_COMMON_TIER0_TLS_H
#pragma once

/*
================
CypherCommon TLS

Thread-local storage declarations.
================
*/

#include "CypherCommon_BaseTypes.h"

namespace cypher::common
{

using tls_slot_t = u32;

constexpr tls_slot_t CY_TLS_INVALID_SLOT = CY_U32_MAX;

tls_slot_t TLS_CreateSlot();
void TLS_DestroySlot( tls_slot_t slot );
void TLS_SetValue( tls_slot_t slot, void *pValue );
void *TLS_GetValue( tls_slot_t slot );

} // namespace cypher::common

#endif // CYPHER_COMMON_TIER0_TLS_H

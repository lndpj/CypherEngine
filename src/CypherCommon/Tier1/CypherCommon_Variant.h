#ifndef CYPHER_COMMON_TIER1_VARIANT_H
#define CYPHER_COMMON_TIER1_VARIANT_H
#pragma once

/*
================
CypherCommon Variant

Small runtime variant declarations.
================
*/

#include "CypherCommon_Tier0.h"

namespace cypher::common
{

enum class variant_type_t : u32 {
    Null = 0u,
    Bool,
    I32,
    U32,
    I64,
    U64,
    F32,
    F64,
    String
};

struct variant_t;

variant_type_t Variant_GetType( const variant_t *pVariant );
void Variant_Clear( variant_t *pVariant );

} // namespace cypher::common

#endif // CYPHER_COMMON_TIER1_VARIANT_H

#ifndef CYPHER_COMMON_TIER0_STATS_H
#define CYPHER_COMMON_TIER0_STATS_H
#pragma once

/*
================
CypherCommon Stats

Named runtime statistics declarations for memory, VFS, pak loading, renderer,
networking and tooling diagnostics.
================
*/

#include "CypherCommon_BaseTypes.h"

namespace cypher::common
{

enum class stat_value_type_t : u8 {
    I64 = 0u,
    U64,
    F64
};

struct stat_value_t {
    stat_value_type_t type;
    union {
        i64 i64Value;
        u64 u64Value;
        f64 f64Value;
    };
};

struct stat_desc_t {
    const char *pName;
    const char *pCategory;
    const char *pDescription;
    stat_value_type_t type;
};

void Cy_StatsRegister( const stat_desc_t &desc );
void Cy_StatsSetI64( const char *pName, i64 value );
void Cy_StatsSetU64( const char *pName, u64 value );
void Cy_StatsSetF64( const char *pName, f64 value );
bool_t Cy_StatsGet( const char *pName, stat_value_t *pOutValue );
void Cy_StatsReset();

} // namespace cypher::common

#endif // CYPHER_COMMON_TIER0_STATS_H

#ifndef CYPHER_COMMON_TIER1_RESULT_H
#define CYPHER_COMMON_TIER1_RESULT_H
#pragma once

/*
================
CypherCommon Result

Generic result declarations.
================
*/

#include "CypherCommon_Tier0.h"

namespace cypher::common
{

enum class result_code_t : u32 {
    Ok = 0u,
    Failed,
    InvalidArgument,
    OutOfMemory,
    NotFound
};

struct result_t {
    result_code_t code;
};

bool_t Result_Succeeded( result_t result );
bool_t Result_Failed( result_t result );

} // namespace cypher::common

#endif // CYPHER_COMMON_TIER1_RESULT_H

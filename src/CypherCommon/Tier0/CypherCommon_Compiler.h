#ifndef CYPHER_COMMON_TIER0_COMPILER_H
#define CYPHER_COMMON_TIER0_COMPILER_H
#pragma once

/*
================
CypherCommon Compiler

Compiler identity and feature declarations.
================
*/

#include "CypherCommon_BaseTypes.h"

namespace cypher::common
{

struct compiler_info_t {
    const char *pName;
    u32 version;
    bool_t has_exceptions;
    bool_t has_rtti;
};

compiler_info_t Compiler_GetInfo();
const char *Compiler_GetName();
u32 Compiler_GetVersion();

} // namespace cypher::common

#endif // CYPHER_COMMON_TIER0_COMPILER_H

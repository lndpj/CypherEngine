#ifndef CYPHER_COMMON_TIER0_MINIDUMP_H
#define CYPHER_COMMON_TIER0_MINIDUMP_H
#pragma once

/*
================
CypherCommon Minidump

Crash dump declarations for supported platforms.
================
*/

#include "CypherCommon_BaseTypes.h"

namespace cypher::common
{

struct minidump_info_t {
    const char *pApplicationName;
    const char *pVersion;
    const char *pOutputPath;
};

bool_t Minidump_Write( const minidump_info_t &info );
void Minidump_SetOutputPath( const char *pPath );

} // namespace cypher::common

#endif // CYPHER_COMMON_TIER0_MINIDUMP_H

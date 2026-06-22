#ifndef CYPHER_COMMON_TIER0_DYNAMICLIBRARY_H
#define CYPHER_COMMON_TIER0_DYNAMICLIBRARY_H
#pragma once

/*
================
CypherCommon Dynamic Library

Runtime shared library declarations.
================
*/

#include "CypherCommon_BaseTypes.h"

namespace cypher::common
{

struct dynamic_library_t {
    void *pHandle;
};

bool_t DynamicLibrary_Load( dynamic_library_t *pLibrary, const char *pPath );
void DynamicLibrary_Unload( dynamic_library_t *pLibrary );
void *DynamicLibrary_GetSymbol( dynamic_library_t *pLibrary, const char *pSymbolName );

} // namespace cypher::common

#endif // CYPHER_COMMON_TIER0_DYNAMICLIBRARY_H

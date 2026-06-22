#ifndef CYPHER_COMMON_TIER0_MODULE_H
#define CYPHER_COMMON_TIER0_MODULE_H
#pragma once

/*
================
CypherCommon Module

Runtime module metadata and lifecycle declarations for engine libraries, game
DLLs, tools and editor plugins.
================
*/

#include "CypherCommon_BaseTypes.h"
#include "CypherCommon_Error.h"

namespace cypher::common
{

enum class module_state_t : u8 {
    Unloaded = 0u,
    Loaded,
    Initialized,
    Shutdown
};

struct module_version_t {
    u32 major;
    u32 minor;
    u32 patch;
    u32 build;
};

struct module_desc_t {
    const char *pName;
    const char *pInternalName;
    const char *pDescription;
    module_version_t version;
    u32 apiVersion;
};

using module_init_fn_t = error_t ( * )( void *pUserData );
using module_shutdown_fn_t = void ( * )( void *pUserData );

const char *Cy_ModuleStateName( module_state_t state );
bool_t Cy_ModuleVersionCompatible( const module_version_t &required, const module_version_t &provided );

} // namespace cypher::common

#endif // CYPHER_COMMON_TIER0_MODULE_H

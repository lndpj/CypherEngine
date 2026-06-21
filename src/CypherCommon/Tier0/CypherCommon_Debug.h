#ifndef CYPHER_COMMON_TIER0_DEBUG_H
#define CYPHER_COMMON_TIER0_DEBUG_H
#pragma once

/*
================
CypherCommon Debug

Lowest-level debug break and trap helpers.

Rules:
- No logging.
- No allocation.
- No CypherEngine dependency.
================
*/

#include "CypherCommon_Platform.h"

#if CYPHER_COMPILER_MSVC
    #include <intrin.h>
#endif

/*
================
Debugger Break / Trap
================
*/
#if CYPHER_COMPILER_MSVC
    #define CYPHER_DEBUG_BREAK()            __debugbreak()
    #define CYPHER_TRAP()                   __debugbreak()
#elif CYPHER_COMPILER_CLANG
    #define CYPHER_DEBUG_BREAK()            __builtin_debugtrap()
    #define CYPHER_TRAP()                   __builtin_trap()
#elif CYPHER_COMPILER_GCC
    #define CYPHER_DEBUG_BREAK()            __builtin_trap()
    #define CYPHER_TRAP()                   __builtin_trap()
#else
    #error "Unsupported compiler for Cypher debug helpers."
#endif

/*
================
Build Gated Code Helpers
================
*/
#if CYPHER_BUILD_DEBUG
    #define CYPHER_DEBUG_ONLY( statement )  do { statement; } while ( 0 )
    #define CYPHER_RELEASE_ONLY( statement ) do { } while ( 0 )
#else
    #define CYPHER_DEBUG_ONLY( statement )  do { } while ( 0 )
    #define CYPHER_RELEASE_ONLY( statement ) do { statement; } while ( 0 )
#endif

#endif // CYPHER_COMMON_TIER0_DEBUG_H

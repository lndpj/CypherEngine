#ifndef CYPHER_COMMON_TIER0_API_H
#define CYPHER_COMMON_TIER0_API_H
#pragma once

/*
================
CypherCommon API

DLL/shared-library visibility macros used when Common, engine runtime, tools
or game modules need a stable binary boundary.
================
*/

#include "CypherCommon_Platform.h"

#if CYPHER_PLATFORM_WINDOWS
    #define CYPHER_API_EXPORT __declspec( dllexport )
    #define CYPHER_API_IMPORT __declspec( dllimport )
    #define CYPHER_API_LOCAL
#elif CYPHER_COMPILER_CLANG || CYPHER_COMPILER_GCC
    #define CYPHER_API_EXPORT __attribute__(( visibility( "default" ) ))
    #define CYPHER_API_IMPORT __attribute__(( visibility( "default" ) ))
    #define CYPHER_API_LOCAL  __attribute__(( visibility( "hidden" ) ))
#else
    #define CYPHER_API_EXPORT
    #define CYPHER_API_IMPORT
    #define CYPHER_API_LOCAL
#endif

#if defined( CYPHER_COMMON_BUILD_DLL )
    #define CYPHER_COMMON_API CYPHER_API_EXPORT
#elif defined( CYPHER_COMMON_USE_DLL )
    #define CYPHER_COMMON_API CYPHER_API_IMPORT
#else
    #define CYPHER_COMMON_API
#endif

#endif // CYPHER_COMMON_TIER0_API_H

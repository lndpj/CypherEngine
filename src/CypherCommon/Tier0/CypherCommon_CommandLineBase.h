#ifndef CYPHER_COMMON_TIER0_COMMANDLINEBASE_H
#define CYPHER_COMMON_TIER0_COMMANDLINEBASE_H
#pragma once

/*
================
CypherCommon Command Line Base

Low-level process command line declarations.
================
*/

#include "CypherCommon_BaseTypes.h"

namespace cypher::common
{

struct command_line_base_t;

void CommandLineBase_Set( command_line_base_t *pCommandLine, i32 argc, const char **ppArgv );
const char *CommandLineBase_Find( const command_line_base_t *pCommandLine, const char *pName );
bool_t CommandLineBase_Has( const command_line_base_t *pCommandLine, const char *pName );

} // namespace cypher::common

#endif // CYPHER_COMMON_TIER0_COMMANDLINEBASE_H

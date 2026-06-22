#ifndef CYPHER_COMMON_TIER1_COMMANDLINE_H
#define CYPHER_COMMON_TIER1_COMMANDLINE_H
#pragma once

/*
================
CypherCommon Command Line

Command-line parser declarations.
================
*/

#include "CypherCommon_Tier0.h"

namespace cypher::common
{

struct command_line_t;

bool_t CommandLine_Init( command_line_t *pCommandLine, i32 argc, const char **ppArgv );
bool_t CommandLine_HasSwitch( const command_line_t *pCommandLine, const char *pName );
const char *CommandLine_GetValue( const command_line_t *pCommandLine, const char *pName );

} // namespace cypher::common

#endif // CYPHER_COMMON_TIER1_COMMANDLINE_H

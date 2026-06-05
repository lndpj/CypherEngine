#pragma once

#include "CypherEngine/CypherCommand/CypherCommand_Error.h"
#include "CypherEngine/CypherCommon/CypherCommon.h"

#define CYPHER_COMMAND_MAX_COMMANDS        256u
#define CYPHER_COMMAND_MAX_ARGUMENTS       16u

namespace cypher::engine::cmd
{

/*
================
Command Types

Commands bind a text name to a callback used by configs, console and tools.
================
*/
using cypher_command_fn_t = void (*)( void *extra_data, common::u32 argc, char **argv );

struct cmd_t {
    const char  *name;
    cypher_command_fn_t    callback_fn;
    void        *extra_data;
    const char  *description;
};

struct cypher_command_registry_t {
    cmd_t cmd_commands[CYPHER_COMMAND_MAX_COMMANDS];
    common::u32 cmd_count;
    bool initialized;
};

/*
================
Command API
================
*/
cypher_command_error_code_t CypherCommand_Init( );

void CypherCommand_Shutdown();

cypher_command_error_code_t CypherCommand_Register( const char *cmd_name, cypher_command_fn_t callback_fn, void *extra_data, const char *cmd_description );

const cmd_t *CypherCommand_Find( const char *cmd_name );

cypher_command_error_code_t CypherCommand_Parse( char *command_line, common::u32 &argc, char **argv );

cypher_command_error_code_t CypherCommand_Execute( const char *command_line );

}

#pragma once

#include "rengine/cmd/cmd_error.h"
#include "rengine/rcommon/com_main.h"

#define CMD_MAX_COMMANDS        256u
#define CMD_MAX_ARGUMENTS       16u

namespace reap::rengine::cmd
{

/*
================
Command Types

Commands bind a text name to a callback used by configs, console and tools.
================
*/
using cmd_fn_t = void (*)( void *extra_data, rcommon::u32 argc, char **argv );

struct cmd_t {
    const char  *name;
    cmd_fn_t    callback_fn;
    void        *extra_data;
    const char  *description;
};

struct cmd_registry_t {
    cmd_t cmd_commands[CMD_MAX_COMMANDS];
    rcommon::u32 cmd_count;
    bool initialized;
};

/*
================
Command API
================
*/
cmd_error_code_t Cmd_Init( );

void Cmd_Shutdown();

cmd_error_code_t Cmd_Register( const char *cmd_name, cmd_fn_t callback_fn, void *extra_data, const char *cmd_description );

const cmd_t *Cmd_Find( const char *cmd_name );

cmd_error_code_t Cmd_Parse( char *command_line, rcommon::u32 &argc, char **argv );

cmd_error_code_t Cmd_Execute( const char *command_line );

}

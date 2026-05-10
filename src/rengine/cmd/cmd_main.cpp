/*======================================================================
   File: cmd_main.cpp
   Project: REAP
   Author: ksiric <email@example.com>
   Created: 2026-04-21 22:26:01
   Last Modified by: ksiric
   Last Modified: 2026-05-04 01:35:52
   ---------------------------------------------------------------------
   Description:

   ---------------------------------------------------------------------
   License:
   Company:
   Version: 0.1.0
 ======================================================================
                                                                       */

#include "rengine/cmd/cmd_main.h"
#include "rengine/cmd/cmd_error.h"
#include "rengine/rcommon/com_print.h"

#include <cctype>      // std::isspace while tokenizing command lines.
#include <cstring>     // strcmp / strncpy for fixed command strings.

namespace reap::rengine::cmd
{

cmd_registry_t g_cmd_registery{};

/*
================
Cmd_Init
================
*/
cmd_error_code_t Cmd_Init( ) {
    if ( g_cmd_registery.initialized ) {
        rcommon::Com_Printf( "Cmd_Init: command system already initialized." );
        return cmd::cmd_error_code_t::ERR_IS_INIT;
    }

    g_cmd_registery = {};

    g_cmd_registery.cmd_count = 0;
    g_cmd_registery.initialized = true;

    return cmd::cmd_error_code_t::OK;
}

/*
================
Cmd_Shutdown
================
*/
void Cmd_Shutdown() {
    if ( !g_cmd_registery.initialized ) {
        rcommon::Com_Printf( "Cmd_Shutdown: command system is not initialized; nothing to shutdown" );
        return ;
    }

    g_cmd_registery = {};
    g_cmd_registery.cmd_count = 0;
    g_cmd_registery.initialized = false;

    return ;
}

/*
================
Cmd_Register

Adds a named command callback to the fixed registry.
================
*/
cmd_error_code_t Cmd_Register( const char *cmd_name, cmd_fn_t callback_fn, void *extra_data, const char *cmd_description ) {
    if ( !g_cmd_registery.initialized ) {
        rcommon::Com_Printf( "Cmd_Register: command system is not initialized." );
        return cmd::cmd_error_code_t::ERR_NOT_INIT;
    }

    if ( cmd_name == nullptr || cmd_name[0] == '\0' ) {
        rcommon::Com_Printf( "Cmd_Register: invalid cmd passed to registery." );
        return cmd::cmd_error_code_t::ERR_INVALID_COMMAND;
    }

    const cmd_t *command = Cmd_Find( cmd_name );

    if ( command != nullptr ) {
        rcommon::Com_Printf( "Cmd_Register: cmd '%s' already exists and is registered.", cmd_name );
        return cmd::cmd_error_code_t::ERR_COMMAND_ALREADY_EXISTS;
    }

    if ( callback_fn == nullptr ) {
        rcommon::Com_Printf( "Cmd_Register: invalid cmd callback passed to registery." );
        return cmd::cmd_error_code_t::ERR_INVALID_CALLBACK;
    }

    rcommon::u32 count = g_cmd_registery.cmd_count;

    if ( g_cmd_registery.cmd_count >= CMD_MAX_COMMANDS ) {
        rcommon::Com_Printf( "Cmd_Register: cannot register new cmd, registery is full." );
        return cmd::cmd_error_code_t::ERR_REGISTRY_FULL;
    }

    g_cmd_registery.cmd_commands[count].name = cmd_name;
    g_cmd_registery.cmd_commands[count].callback_fn = callback_fn;
    g_cmd_registery.cmd_commands[count].extra_data = extra_data;
    g_cmd_registery.cmd_commands[count].description = cmd_description;
    count++;

    g_cmd_registery.cmd_count = count;

    return cmd::cmd_error_code_t::OK;
}

/*
================
Cmd_Find
================
*/
const cmd_t *Cmd_Find( const char *cmd_name ) {
    if ( !g_cmd_registery.initialized ) {
        rcommon::Com_Printf( "Cmd_Find: cmd system is not initialized." );
        return nullptr;
    }

    if ( cmd_name == nullptr || cmd_name[0] == '\0' ) {
        rcommon::Com_Printf( "Cmd_Find: invalid cmd name passed to find." );
        return nullptr;
    }

    for ( rcommon::u32 i = 0; i < g_cmd_registery.cmd_count; i++ ) {
        if ( std::strcmp( cmd_name, g_cmd_registery.cmd_commands[i].name ) == 0 ) {
            return &g_cmd_registery.cmd_commands[i];
        }
    }

    return nullptr;
}

/*
================
Cmd_Parse

Splits a mutable command line into argv-style tokens.
================
*/
cmd_error_code_t Cmd_Parse( char *command_line, rcommon::u32 &argc, char **argv ) {

    if ( command_line == nullptr || command_line[0] == '\0' ) {
        rcommon::Com_Printf( "Cmd_Parse: invalid command line passed for parsing." );
        return cmd::cmd_error_code_t::ERR_INVALID_COMMAND;
    }

    argc = 0;

    char *cursor_ptr = command_line;
    
    while ( *cursor_ptr != '\0' ) {
        while ( *cursor_ptr != '\0' && std::isspace( static_cast<unsigned char>( *cursor_ptr ) ) ) {
            cursor_ptr++;
        }
        if ( *cursor_ptr == '\0' ) {
            break;
        }
        if ( argc >= CMD_MAX_ARGUMENTS ) {
            return cmd::cmd_error_code_t::ERR_PARSE_FAILED;
        }
        argv[argc++] = cursor_ptr;
        while( *cursor_ptr != '\0' && !std::isspace( static_cast<unsigned char>( *cursor_ptr ) ) ) {
            cursor_ptr++;
        }
        
        if ( *cursor_ptr == '\0' ) {
            break;
        }

        *cursor_ptr = '\0';
        cursor_ptr++;
    }

    return ( argc > 0u ) ? cmd_error_code_t::OK : cmd_error_code_t::ERR_INVALID_COMMAND;
}

/*
================
Cmd_Execute

Parses a command line, finds the command, and calls its callback.
================
*/
cmd_error_code_t Cmd_Execute( const char *command_line ) {
    if ( !g_cmd_registery.initialized ) {
        rcommon::Com_Printf( "Cmd_Execute: cmd system is not initialized; nothing to execute." );
        return cmd::cmd_error_code_t::ERR_NOT_INIT;
    }

    if ( command_line == nullptr || command_line[0] == '\0' ) {
        rcommon::Com_Printf( "Cmd_Execute: invalid command line passed to execute." );
        return cmd::cmd_error_code_t::ERR_INVALID_COMMAND;
    }

    rcommon::u32 cmd_argc{};
    char *cmd_argv[CMD_MAX_ARGUMENTS]{};

    char buffer[1024]{};

    strncpy( buffer, command_line, sizeof( buffer ) - 1 );

    cmd_error_code_t err = Cmd_Parse( buffer, cmd_argc, cmd_argv );
    
    if ( err != cmd::cmd_error_code_t::OK ) {
        rcommon::Com_Errorf( Cmd_ErrorCode( err ), "Cmd_Execute: Cmd_Parse: invalid parsing command line." );
    }
    
    if ( cmd_argc == 0u || cmd_argv[0] == nullptr || cmd_argv[0][0] == '\0' ) {
        rcommon::Com_Printf( "Cmd_Execute: parsed command line is empty." );
        return cmd::cmd_error_code_t::ERR_INVALID_COMMAND;
    }
    
    const cmd_t *cmd = Cmd_Find( cmd_argv[0] );
    if ( cmd == nullptr ) {
       rcommon::Com_Printf( "Cmd_Execute: command '%s' not found.", cmd_argv[0] );
        return cmd::cmd_error_code_t::ERR_COMMAND_NOT_FOUND; 
    }
    
    if ( cmd->callback_fn == nullptr ) {
        rcommon::Com_Printf( "Cmd_Execute: command '%s' has invalid callback.", cmd_argv[0] );
        return cmd::cmd_error_code_t::ERR_INVALID_CALLBACK;
    }
    
    cmd->callback_fn( cmd->extra_data, cmd_argc, cmd_argv );
    return cmd_error_code_t::OK;
}

} // namespace reap::rengine::cmd

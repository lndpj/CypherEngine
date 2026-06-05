/*======================================================================
   File: CypherCommand.cpp
   Project: CypherEngine
   Author: ksiric <email@example.com>
   Created: 2026-04-21 22:26:01
   Last Modified by: ksiric
   Last Modified: 2026-06-05 12:00:30
   ---------------------------------------------------------------------
   Description:

   ---------------------------------------------------------------------
   License:
   Company:
   Version: 0.1.0
 ======================================================================
                                                                       */

#include "CypherEngine/CypherCommand/CypherCommand.h"
#include "CypherEngine/CypherCommand/CypherCommand_Error.h"
#include "CypherEngine/CypherCommon/CypherCommon_Print.h"

#include <cctype>      // std::isspace while tokenizing command lines.
#include <cstring>     // strcmp / strncpy for fixed command strings.

namespace cypher::engine::cmd
{

cypher_command_registry_t g_cmd_registery{};

/*
================
CypherCommand_Init
================
*/
cypher_command_error_code_t CypherCommand_Init( ) {
    if ( g_cmd_registery.initialized ) {
        common::CypherCommon_Printf( "CypherCommand_Init: command system already initialized." );
        return cmd::cypher_command_error_code_t::ERR_IS_INIT;
    }

    g_cmd_registery = {};

    g_cmd_registery.cmd_count = 0;
    g_cmd_registery.initialized = true;

    return cmd::cypher_command_error_code_t::OK;
}

/*
================
CypherCommand_Shutdown
================
*/
void CypherCommand_Shutdown() {
    if ( !g_cmd_registery.initialized ) {
        common::CypherCommon_Printf( "CypherCommand_Shutdown: command system is not initialized; nothing to shutdown" );
        return ;
    }

    g_cmd_registery = {};
    g_cmd_registery.cmd_count = 0;
    g_cmd_registery.initialized = false;

    return ;
}

/*
================
CypherCommand_Register

Adds a named command callback to the fixed registry.
================
*/
cypher_command_error_code_t CypherCommand_Register( const char *cmd_name, cypher_command_fn_t callback_fn, void *extra_data, const char *cmd_description ) {
    if ( !g_cmd_registery.initialized ) {
        common::CypherCommon_Printf( "CypherCommand_Register: command system is not initialized." );
        return cmd::cypher_command_error_code_t::ERR_NOT_INIT;
    }

    if ( cmd_name == nullptr || cmd_name[0] == '\0' ) {
        common::CypherCommon_Printf( "CypherCommand_Register: invalid cmd passed to registery." );
        return cmd::cypher_command_error_code_t::ERR_INVALID_COMMAND;
    }

    const cmd_t *command = CypherCommand_Find( cmd_name );

    if ( command != nullptr ) {
        common::CypherCommon_Printf( "CypherCommand_Register: cmd '%s' already exists and is registered.", cmd_name );
        return cmd::cypher_command_error_code_t::ERR_COMMAND_ALREADY_EXISTS;
    }

    if ( callback_fn == nullptr ) {
        common::CypherCommon_Printf( "CypherCommand_Register: invalid cmd callback passed to registery." );
        return cmd::cypher_command_error_code_t::ERR_INVALID_CALLBACK;
    }

    common::u32 count = g_cmd_registery.cmd_count;

    if ( g_cmd_registery.cmd_count >= CYPHER_COMMAND_MAX_COMMANDS ) {
        common::CypherCommon_Printf( "CypherCommand_Register: cannot register new cmd, registery is full." );
        return cmd::cypher_command_error_code_t::ERR_REGISTRY_FULL;
    }

    g_cmd_registery.cmd_commands[count].name = cmd_name;
    g_cmd_registery.cmd_commands[count].callback_fn = callback_fn;
    g_cmd_registery.cmd_commands[count].extra_data = extra_data;
    g_cmd_registery.cmd_commands[count].description = cmd_description;
    count++;

    g_cmd_registery.cmd_count = count;

    return cmd::cypher_command_error_code_t::OK;
}

/*
================
CypherCommand_Find
================
*/
const cmd_t *CypherCommand_Find( const char *cmd_name ) {
    if ( !g_cmd_registery.initialized ) {
        common::CypherCommon_Printf( "CypherCommand_Find: cmd system is not initialized." );
        return nullptr;
    }

    if ( cmd_name == nullptr || cmd_name[0] == '\0' ) {
        common::CypherCommon_Printf( "CypherCommand_Find: invalid cmd name passed to find." );
        return nullptr;
    }

    for ( common::u32 i = 0; i < g_cmd_registery.cmd_count; i++ ) {
        if ( std::strcmp( cmd_name, g_cmd_registery.cmd_commands[i].name ) == 0 ) {
            return &g_cmd_registery.cmd_commands[i];
        }
    }

    return nullptr;
}

/*
================
CypherCommand_Parse

Splits a mutable command line into argv-style tokens.
================
*/
cypher_command_error_code_t CypherCommand_Parse( char *command_line, common::u32 &argc, char **argv ) {

    if ( command_line == nullptr || command_line[0] == '\0' ) {
        common::CypherCommon_Printf( "CypherCommand_Parse: invalid command line passed for parsing." );
        return cmd::cypher_command_error_code_t::ERR_INVALID_COMMAND;
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
        if ( argc >= CYPHER_COMMAND_MAX_ARGUMENTS ) {
            return cmd::cypher_command_error_code_t::ERR_PARSE_FAILED;
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

    return ( argc > 0u ) ? cypher_command_error_code_t::OK : cypher_command_error_code_t::ERR_INVALID_COMMAND;
}

/*
================
CypherCommand_Execute

Parses a command line, finds the command, and calls its callback.
================
*/
cypher_command_error_code_t CypherCommand_Execute( const char *command_line ) {
    if ( !g_cmd_registery.initialized ) {
        common::CypherCommon_Printf( "CypherCommand_Execute: cmd system is not initialized; nothing to execute." );
        return cmd::cypher_command_error_code_t::ERR_NOT_INIT;
    }

    if ( command_line == nullptr || command_line[0] == '\0' ) {
        common::CypherCommon_Printf( "CypherCommand_Execute: invalid command line passed to execute." );
        return cmd::cypher_command_error_code_t::ERR_INVALID_COMMAND;
    }

    common::u32 cmd_argc{};
    char *cmd_argv[CYPHER_COMMAND_MAX_ARGUMENTS]{};

    char buffer[1024]{};

    strncpy( buffer, command_line, sizeof( buffer ) - 1 );

    cypher_command_error_code_t err = CypherCommand_Parse( buffer, cmd_argc, cmd_argv );
    
    if ( err != cmd::cypher_command_error_code_t::OK ) {
        common::CypherCommon_Errorf( CypherCommand_ErrorCode( err ), "CypherCommand_Execute: CypherCommand_Parse: invalid parsing command line." );
    }
    
    if ( cmd_argc == 0u || cmd_argv[0] == nullptr || cmd_argv[0][0] == '\0' ) {
        common::CypherCommon_Printf( "CypherCommand_Execute: parsed command line is empty." );
        return cmd::cypher_command_error_code_t::ERR_INVALID_COMMAND;
    }
    
    const cmd_t *cmd = CypherCommand_Find( cmd_argv[0] );
    if ( cmd == nullptr ) {
       common::CypherCommon_Printf( "CypherCommand_Execute: command '%s' not found.", cmd_argv[0] );
        return cmd::cypher_command_error_code_t::ERR_COMMAND_NOT_FOUND; 
    }
    
    if ( cmd->callback_fn == nullptr ) {
        common::CypherCommon_Printf( "CypherCommand_Execute: command '%s' has invalid callback.", cmd_argv[0] );
        return cmd::cypher_command_error_code_t::ERR_INVALID_CALLBACK;
    }
    
    cmd->callback_fn( cmd->extra_data, cmd_argc, cmd_argv );
    return cypher_command_error_code_t::OK;
}

} // namespace cypher::engine::cmd

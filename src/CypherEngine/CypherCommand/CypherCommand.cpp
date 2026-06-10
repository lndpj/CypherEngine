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
#include "CypherEngine/CypherLog/CypherLog.h"

#include <cctype>      // std::isspace while tokenizing command lines.
#include <cstring>     // strcmp / strncpy for fixed command strings.

namespace cypher::engine::cmd
{

registry_t g_cmd_registery{};

/*
================
CypherCommand_Init
================
*/
error_code_t CypherCommand_Init( ) {
    if ( g_cmd_registery.initialized ) {
        CYPHER_LOG_WARNING( log::channel_t::CMD, "command system init requested while already initialized." );
        return cmd::error_code_t::ERR_IS_INIT;
    }

    g_cmd_registery = {};

    g_cmd_registery.cmd_count = 0;
    g_cmd_registery.initialized = true;

    CYPHER_LOG_INFO( log::channel_t::CMD, "command system initialized." );

    return cmd::error_code_t::OK;
}

/*
================
CypherCommand_Shutdown
================
*/
void CypherCommand_Shutdown() {
    if ( !g_cmd_registery.initialized ) {
        CYPHER_LOG_WARNING( log::channel_t::CMD, "command system shutdown requested while not initialized." );
        return ;
    }

    CYPHER_LOG_INFO( log::channel_t::CMD, "command system shutdown: commands=%u.", g_cmd_registery.cmd_count );

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
error_code_t CypherCommand_Register( const char *cmd_name, command_fn_t callback_fn, void *extra_data, const char *cmd_description ) {
    if ( !g_cmd_registery.initialized ) {
        CYPHER_LOG_ERROR( log::channel_t::CMD, "command register failed for '%s': command system is not initialized.", cmd_name ? cmd_name : "<null>" );
        return cmd::error_code_t::ERR_NOT_INIT;
    }

    if ( cmd_name == nullptr || cmd_name[0] == '\0' ) {
        CYPHER_LOG_ERROR( log::channel_t::CMD, "command register failed: invalid command name." );
        return cmd::error_code_t::ERR_INVALID_COMMAND;
    }

    const cmd_t *command = CypherCommand_Find( cmd_name );

    if ( command != nullptr ) {
        CYPHER_LOG_WARNING( log::channel_t::CMD, "command register skipped: '%s' already exists.", cmd_name );
        return cmd::error_code_t::ERR_COMMAND_ALREADY_EXISTS;
    }

    if ( callback_fn == nullptr ) {
        CYPHER_LOG_ERROR( log::channel_t::CMD, "command register failed for '%s': invalid callback.", cmd_name );
        return cmd::error_code_t::ERR_INVALID_CALLBACK;
    }

    common::u32 count = g_cmd_registery.cmd_count;

    if ( g_cmd_registery.cmd_count >= CYPHER_COMMAND_MAX_COMMANDS ) {
        CYPHER_LOG_ERROR( log::channel_t::CMD, "command register failed for '%s': registry full (%u).", cmd_name, CYPHER_COMMAND_MAX_COMMANDS );
        return cmd::error_code_t::ERR_REGISTRY_FULL;
    }

    g_cmd_registery.cmd_commands[count].name = cmd_name;
    g_cmd_registery.cmd_commands[count].callback_fn = callback_fn;
    g_cmd_registery.cmd_commands[count].extra_data = extra_data;
    g_cmd_registery.cmd_commands[count].description = cmd_description;
    count++;

    g_cmd_registery.cmd_count = count;

    CYPHER_LOG_DEBUG( log::channel_t::CMD, "registered command '%s'.", cmd_name );

    return cmd::error_code_t::OK;
}

/*
================
CypherCommand_Find
================
*/
const cmd_t *CypherCommand_Find( const char *cmd_name ) {
    if ( !g_cmd_registery.initialized ) {
        CYPHER_LOG_ERROR( log::channel_t::CMD, "command find failed for '%s': command system is not initialized.", cmd_name ? cmd_name : "<null>" );
        return nullptr;
    }

    if ( cmd_name == nullptr || cmd_name[0] == '\0' ) {
        CYPHER_LOG_ERROR( log::channel_t::CMD, "command find failed: invalid command name." );
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
error_code_t CypherCommand_Parse( char *command_line, common::u32 &argc, char **argv ) {

    if ( command_line == nullptr || command_line[0] == '\0' ) {
        CYPHER_LOG_ERROR( log::channel_t::CMD, "command parse failed: invalid command line." );
        return cmd::error_code_t::ERR_INVALID_COMMAND;
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
            return cmd::error_code_t::ERR_PARSE_FAILED;
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

    return ( argc > 0u ) ? error_code_t::OK : error_code_t::ERR_INVALID_COMMAND;
}

/*
================
CypherCommand_Execute

Parses a command line, finds the command, and calls its callback.
================
*/
error_code_t CypherCommand_Execute( const char *command_line ) {
    if ( !g_cmd_registery.initialized ) {
        CYPHER_LOG_ERROR( log::channel_t::CMD, "command execute failed: command system is not initialized." );
        return cmd::error_code_t::ERR_NOT_INIT;
    }

    if ( command_line == nullptr || command_line[0] == '\0' ) {
        CYPHER_LOG_ERROR( log::channel_t::CMD, "command execute failed: invalid command line." );
        return cmd::error_code_t::ERR_INVALID_COMMAND;
    }

    common::u32 cmd_argc{};
    char *cmd_argv[CYPHER_COMMAND_MAX_ARGUMENTS]{};

    char buffer[1024]{};

    strncpy( buffer, command_line, sizeof( buffer ) - 1 );

    error_code_t err = CypherCommand_Parse( buffer, cmd_argc, cmd_argv );
    
    if ( err != cmd::error_code_t::OK ) {
        common::CypherCommon_Errorf( CypherCommand_ErrorCode( err ), "CypherCommand_Execute: CypherCommand_Parse: invalid parsing command line." );
        CYPHER_LOG_ERROR( log::channel_t::CMD, "command execute failed: parse failed for '%s'.", command_line );
        return err;
    }
    
    if ( cmd_argc == 0u || cmd_argv[0] == nullptr || cmd_argv[0][0] == '\0' ) {
        CYPHER_LOG_WARNING( log::channel_t::CMD, "command execute skipped: parsed command line is empty." );
        return cmd::error_code_t::ERR_INVALID_COMMAND;
    }
    
    const cmd_t *cmd = CypherCommand_Find( cmd_argv[0] );
    if ( cmd == nullptr ) {
        CYPHER_LOG_WARNING( log::channel_t::CMD, "command execute failed: command '%s' not found.", cmd_argv[0] );
        return cmd::error_code_t::ERR_COMMAND_NOT_FOUND; 
    }
    
    if ( cmd->callback_fn == nullptr ) {
        CYPHER_LOG_ERROR( log::channel_t::CMD, "command execute failed: command '%s' has invalid callback.", cmd_argv[0] );
        return cmd::error_code_t::ERR_INVALID_CALLBACK;
    }
    
    cmd->callback_fn( cmd->extra_data, cmd_argc, cmd_argv );
    return error_code_t::OK;
}

} // namespace cypher::engine::cmd

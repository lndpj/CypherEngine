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

registry_t g_CmdRegistery{};

/*
================
CypherCommand_Init
================
*/
cmd_error_t CypherCommand_Init( ) {
    if ( g_CmdRegistery.initialized ) {
        LOG_WARNING( log::channel_t::CMD, "command system init requested while already initialized." );
        return cmd_error_t::ERR_IS_INIT;
    }

    g_CmdRegistery = {};

    g_CmdRegistery.nCmdCount = 0;
    g_CmdRegistery.initialized = true;

    LOG_INFO( log::channel_t::CMD, "command system initialized." );

    return cmd_error_t::OK;
}

/*
================
CypherCommand_Shutdown
================
*/
void CypherCommand_Shutdown() {
    if ( !g_CmdRegistery.initialized ) {
        LOG_WARNING( log::channel_t::CMD, "command system shutdown requested while not initialized." );
        return ;
    }

    LOG_INFO( log::channel_t::CMD, "command system shutdown: commands=%u.", g_CmdRegistery.nCmdCount );

    g_CmdRegistery = {};
    g_CmdRegistery.nCmdCount = 0;
    g_CmdRegistery.initialized = false;

    return ;
}

/*
================
CypherCommand_Register

Adds a named command callback to the fixed registry.
================
*/
cmd_error_t CypherCommand_Register( const char *szCmdName, command_fn_t pCallbackFn, void *pExtraData, const char *szCmdDescription ) {
    if ( !g_CmdRegistery.initialized ) {
        LOG_ERROR( log::channel_t::CMD, "command register failed for '%s': command system is not initialized.", szCmdName ? szCmdName : "<null>" );
        return cmd_error_t::ERR_NOT_INIT;
    }

    if ( szCmdName == nullptr || szCmdName[0] == '\0' ) {
        LOG_ERROR( log::channel_t::CMD, "command register failed: invalid command name." );
        return cmd_error_t::ERR_INVALID_COMMAND;
    }

    const cmd_t *command = CypherCommand_Find( szCmdName );

    if ( command != nullptr ) {
        LOG_WARNING( log::channel_t::CMD, "command register skipped: '%s' already exists.", szCmdName );
        return cmd_error_t::ERR_COMMAND_ALREADY_EXISTS;
    }

    if ( pCallbackFn == nullptr ) {
        LOG_ERROR( log::channel_t::CMD, "command register failed for '%s': invalid callback.", szCmdName );
        return cmd_error_t::ERR_INVALID_CALLBACK;
    }

    common::u32 count = g_CmdRegistery.nCmdCount;

    if ( g_CmdRegistery.nCmdCount >= CYPHER_COMMAND_MAX_COMMANDS ) {
        LOG_ERROR( log::channel_t::CMD, "command register failed for '%s': registry full (%u).", szCmdName, CYPHER_COMMAND_MAX_COMMANDS );
        return cmd_error_t::ERR_REGISTRY_FULL;
    }

    g_CmdRegistery.cmdCommands[count].name = szCmdName;
    g_CmdRegistery.cmdCommands[count].pCallbackFn = pCallbackFn;
    g_CmdRegistery.cmdCommands[count].pExtraData = pExtraData;
    g_CmdRegistery.cmdCommands[count].description = szCmdDescription;
    count++;

    g_CmdRegistery.nCmdCount = count;

    LOG_DEBUG( log::channel_t::CMD, "registered command '%s'.", szCmdName );

    return cmd_error_t::OK;
}

/*
================
CypherCommand_Find
================
*/
const cmd_t *CypherCommand_Find( const char *szCmdName ) {
    if ( !g_CmdRegistery.initialized ) {
        LOG_ERROR( log::channel_t::CMD, "command find failed for '%s': command system is not initialized.", szCmdName ? szCmdName : "<null>" );
        return nullptr;
    }

    if ( szCmdName == nullptr || szCmdName[0] == '\0' ) {
        LOG_ERROR( log::channel_t::CMD, "command find failed: invalid command name." );
        return nullptr;
    }

    for ( common::u32 i = 0; i < g_CmdRegistery.nCmdCount; i++ ) {
        if ( std::strcmp( szCmdName, g_CmdRegistery.cmdCommands[i].name ) == 0 ) {
            return &g_CmdRegistery.cmdCommands[i];
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
cmd_error_t CypherCommand_Parse( char *nCommandLine, common::u32 &argc, char **argv ) {

    if ( nCommandLine == nullptr || nCommandLine[0] == '\0' ) {
        LOG_ERROR( log::channel_t::CMD, "command parse failed: invalid command line." );
        return cmd_error_t::ERR_INVALID_COMMAND;
    }

    argc = 0;

    char *pCursorPtr = nCommandLine;

    while ( *pCursorPtr != '\0' ) {
        while ( *pCursorPtr != '\0' && std::isspace( static_cast<unsigned char>( *pCursorPtr ) ) ) {
            pCursorPtr++;
        }
        if ( *pCursorPtr == '\0' ) {
            break;
        }
        if ( argc >= CYPHER_COMMAND_MAX_ARGUMENTS ) {
            return cmd_error_t::ERR_PARSE_FAILED;
        }
        argv[argc++] = pCursorPtr;
        while( *pCursorPtr != '\0' && !std::isspace( static_cast<unsigned char>( *pCursorPtr ) ) ) {
            pCursorPtr++;
        }

        if ( *pCursorPtr == '\0' ) {
            break;
        }

        *pCursorPtr = '\0';
        pCursorPtr++;
    }

    return ( argc > 0u ) ? cmd_error_t::OK : cmd_error_t::ERR_INVALID_COMMAND;
}

/*
================
CypherCommand_Execute

Parses a command line, finds the command, and calls its callback.
================
*/
cmd_error_t CypherCommand_Execute( const char *nCommandLine ) {
    if ( !g_CmdRegistery.initialized ) {
        LOG_ERROR( log::channel_t::CMD, "command execute failed: command system is not initialized." );
        return cmd_error_t::ERR_NOT_INIT;
    }

    if ( nCommandLine == nullptr || nCommandLine[0] == '\0' ) {
        LOG_ERROR( log::channel_t::CMD, "command execute failed: invalid command line." );
        return cmd_error_t::ERR_INVALID_COMMAND;
    }

    common::u32 nCmdArgc{};
    char *ppszCmdArgv[CYPHER_COMMAND_MAX_ARGUMENTS]{};

    char buffer[1024]{};

    strncpy( buffer, nCommandLine, sizeof( buffer ) - 1 );

    cmd_error_t err = CypherCommand_Parse( buffer, nCmdArgc, ppszCmdArgv );

    if ( err != cmd_error_t::OK ) {
        COM_ERRORF( CypherCommand_ErrorCode( err ), "CypherCommand_Execute: CypherCommand_Parse: invalid parsing command line." );
        LOG_ERROR( log::channel_t::CMD, "command execute failed: parse failed for '%s'.", nCommandLine );
        return err;
    }

    if ( nCmdArgc == 0u || ppszCmdArgv[0] == nullptr || ppszCmdArgv[0][0] == '\0' ) {
        LOG_WARNING( log::channel_t::CMD, "command execute skipped: parsed command line is empty." );
        return cmd_error_t::ERR_INVALID_COMMAND;
    }

    const cmd_t *cmd = CypherCommand_Find( ppszCmdArgv[0] );
    if ( cmd == nullptr ) {
        LOG_WARNING( log::channel_t::CMD, "command execute failed: command '%s' not found.", ppszCmdArgv[0] );
        return cmd_error_t::ERR_COMMAND_NOT_FOUND;
    }

    if ( cmd->pCallbackFn == nullptr ) {
        LOG_ERROR( log::channel_t::CMD, "command execute failed: command '%s' has invalid callback.", ppszCmdArgv[0] );
        return cmd_error_t::ERR_INVALID_CALLBACK;
    }

    cmd->pCallbackFn( cmd->pExtraData, nCmdArgc, ppszCmdArgv );
    return cmd_error_t::OK;
}

} // namespace cypher::engine::cmd

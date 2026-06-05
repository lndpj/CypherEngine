#pragma once

#include "CypherEngine/CypherCommon/CypherCommon_Error.h"
#include "CypherEngine/CypherCommon/CypherCommon.h"

namespace cypher::engine::cmd
{

/*
================
Command Error Codes
================
*/
enum class error_code_t : common::u8 {
    OK,

    ERR_NOT_INIT,
    ERR_IS_INIT,
    ERR_INVALID_COMMAND,
    ERR_COMMAND_ALREADY_EXISTS,
    ERR_COMMAND_NOT_FOUND,
    ERR_REGISTRY_FULL,
    ERR_PARSE_FAILED,
    ERR_INVALID_CALLBACK
};

/*
================
Command Error Helpers
================
*/
constexpr inline const char *CypherCommand_ErrorName( const error_code_t error ) {
    switch ( error ) {
    case error_code_t::OK:
        return "OK";
    case error_code_t::ERR_NOT_INIT:
        return "ERR_NOT_INIT";
    case error_code_t::ERR_IS_INIT:
        return "ERR_IS_INIT";
    case error_code_t::ERR_INVALID_COMMAND:
        return "ERR_INVALID_COMMAND";
    case error_code_t::ERR_COMMAND_ALREADY_EXISTS:
        return "ERR_COMMAND_ALREADY_EXISTS";
    case error_code_t::ERR_COMMAND_NOT_FOUND:
        return "ERR_COMMAND_NOT_FOUND";
    case error_code_t::ERR_REGISTRY_FULL:
        return "ERR_REGISTRY_FULL";
    case error_code_t::ERR_PARSE_FAILED:
        return "ERR_PARSE_FAILED";
    case error_code_t::ERR_INVALID_CALLBACK:
        return "ERR_INVALID_CALLBACK";
    default:
        return "ERR_UNKNOWN";
    }
}

constexpr inline const char *CypherCommand_ErrorDesc( const error_code_t error ) {
    switch ( error ) {
    case error_code_t::OK:
        return "operation completed successfully";
    case error_code_t::ERR_NOT_INIT:
        return "cmd subsystem is not initialized";
    case error_code_t::ERR_IS_INIT:
        return "cmd subsystem is already initialized";
    case error_code_t::ERR_INVALID_COMMAND:
        return "invalid command name or command line";
    case error_code_t::ERR_COMMAND_ALREADY_EXISTS:
        return "command is already registered";
    case error_code_t::ERR_COMMAND_NOT_FOUND:
        return "command was not found";
    case error_code_t::ERR_REGISTRY_FULL:
        return "command registry is full";
    case error_code_t::ERR_PARSE_FAILED:
        return "command line parsing failed";
    case error_code_t::ERR_INVALID_CALLBACK:
        return "invalid command callback";
    default:
        return "unknown cmd error";
    }
}

constexpr inline common::error_t CypherCommand_ErrorCode( error_code_t error ) {
    return common::CypherCommon_ErrorMake( common::domain_t::COM_DOMAIN_CMD , static_cast<common::com_u16>( error ) );
}

}

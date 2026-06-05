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
enum class cypher_command_error_code_t : common::u8 {
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
constexpr inline const char *CypherCommand_ErrorName( const cypher_command_error_code_t error ) {
    switch ( error ) {
    case cypher_command_error_code_t::OK:
        return "OK";
    case cypher_command_error_code_t::ERR_NOT_INIT:
        return "ERR_NOT_INIT";
    case cypher_command_error_code_t::ERR_IS_INIT:
        return "ERR_IS_INIT";
    case cypher_command_error_code_t::ERR_INVALID_COMMAND:
        return "ERR_INVALID_COMMAND";
    case cypher_command_error_code_t::ERR_COMMAND_ALREADY_EXISTS:
        return "ERR_COMMAND_ALREADY_EXISTS";
    case cypher_command_error_code_t::ERR_COMMAND_NOT_FOUND:
        return "ERR_COMMAND_NOT_FOUND";
    case cypher_command_error_code_t::ERR_REGISTRY_FULL:
        return "ERR_REGISTRY_FULL";
    case cypher_command_error_code_t::ERR_PARSE_FAILED:
        return "ERR_PARSE_FAILED";
    case cypher_command_error_code_t::ERR_INVALID_CALLBACK:
        return "ERR_INVALID_CALLBACK";
    default:
        return "ERR_UNKNOWN";
    }
}

constexpr inline const char *CypherCommand_ErrorDesc( const cypher_command_error_code_t error ) {
    switch ( error ) {
    case cypher_command_error_code_t::OK:
        return "operation completed successfully";
    case cypher_command_error_code_t::ERR_NOT_INIT:
        return "cmd subsystem is not initialized";
    case cypher_command_error_code_t::ERR_IS_INIT:
        return "cmd subsystem is already initialized";
    case cypher_command_error_code_t::ERR_INVALID_COMMAND:
        return "invalid command name or command line";
    case cypher_command_error_code_t::ERR_COMMAND_ALREADY_EXISTS:
        return "command is already registered";
    case cypher_command_error_code_t::ERR_COMMAND_NOT_FOUND:
        return "command was not found";
    case cypher_command_error_code_t::ERR_REGISTRY_FULL:
        return "command registry is full";
    case cypher_command_error_code_t::ERR_PARSE_FAILED:
        return "command line parsing failed";
    case cypher_command_error_code_t::ERR_INVALID_CALLBACK:
        return "invalid command callback";
    default:
        return "unknown cmd error";
    }
}

constexpr inline common::cypher_common_error_t CypherCommand_ErrorCode( cypher_command_error_code_t error ) {
    return common::CypherCommon_ErrorMake( common::cypher_common_domain_t::COM_DOMAIN_CMD , static_cast<common::com_u16>( error ) );
}

}

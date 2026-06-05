#pragma once

#include "CypherEngine/CypherCommon/CypherCommon_Error.h"

namespace cypher::engine::sys {

/*
================
System Error Codes
================
*/
enum class cypher_system_error_code_t : common::u8 {
	OK = 0,

	ERR_NOT_INIT,
	ERR_IS_INIT,

	ERR_INVALID_ARGUMENT,
	ERR_INVALID_PATH,

	ERR_UNSUPPORTED_PLATFORM,
	ERR_UNSUPPORTED_COMPILER,

	ERR_PATH_QUERY_FAILED,
	ERR_PATH_TOO_LONG,
	ERR_DIRECTORY_CREATE_FAILED,

	ERR_TIME_UNAVAILABLE,
	ERR_LOCALTIME_FAILED,

	ERR_INTERNAL_ERROR
};

/*
================
System Error Helpers
================
*/
constexpr inline const char *CypherSystem_ErrorName( const cypher_system_error_code_t error ) {
    switch ( error ) {
    case cypher_system_error_code_t::OK:
        return "OK";
    case cypher_system_error_code_t::ERR_NOT_INIT:
        return "ERR_NOT_INIT";
    case cypher_system_error_code_t::ERR_IS_INIT:
        return "ERR_IS_INIT";
    case cypher_system_error_code_t::ERR_INVALID_ARGUMENT:
        return "ERR_INVALID_ARGUMENT";
    case cypher_system_error_code_t::ERR_INVALID_PATH:
        return "ERR_INVALID_PATH";
    case cypher_system_error_code_t::ERR_UNSUPPORTED_PLATFORM:
        return "ERR_UNSUPPORTED_PLATFORM";
    case cypher_system_error_code_t::ERR_UNSUPPORTED_COMPILER:
        return "ERR_UNSUPPORTED_COMPILER";
    case cypher_system_error_code_t::ERR_PATH_QUERY_FAILED:
        return "ERR_PATH_QUERY_FAILED";
    case cypher_system_error_code_t::ERR_PATH_TOO_LONG:
        return "ERR_PATH_TOO_LONG";
    case cypher_system_error_code_t::ERR_DIRECTORY_CREATE_FAILED:
        return "ERR_DIRECTORY_CREATE_FAILED";
    case cypher_system_error_code_t::ERR_TIME_UNAVAILABLE:
        return "ERR_TIME_UNAVAILABLE";
    case cypher_system_error_code_t::ERR_LOCALTIME_FAILED:
        return "ERR_LOCALTIME_FAILED";
    case cypher_system_error_code_t::ERR_INTERNAL_ERROR:
        return "ERR_INTERNAL_ERROR";
    default:
        return "ERR_UNKNOWN";
    }
}

constexpr inline const char *CypherSystem_ErrorDesc( const cypher_system_error_code_t error ) {
    switch ( error ) {
    case cypher_system_error_code_t::OK:
        return "operation completed successfully";
    case cypher_system_error_code_t::ERR_NOT_INIT:
        return "sys subsystem is not initialized";
    case cypher_system_error_code_t::ERR_IS_INIT:
        return "sys subsystem is already initialized";
    case cypher_system_error_code_t::ERR_INVALID_ARGUMENT:
        return "invalid sys argument";
    case cypher_system_error_code_t::ERR_INVALID_PATH:
        return "invalid sys path";
    case cypher_system_error_code_t::ERR_UNSUPPORTED_PLATFORM:
        return "unsupported platform";
    case cypher_system_error_code_t::ERR_UNSUPPORTED_COMPILER:
        return "unsupported compiler";
    case cypher_system_error_code_t::ERR_PATH_QUERY_FAILED:
        return "failed to query platform path";
    case cypher_system_error_code_t::ERR_PATH_TOO_LONG:
        return "platform path is too long";
    case cypher_system_error_code_t::ERR_DIRECTORY_CREATE_FAILED:
        return "failed to create platform directory";
    case cypher_system_error_code_t::ERR_TIME_UNAVAILABLE:
        return "platform time source is unavailable";
    case cypher_system_error_code_t::ERR_LOCALTIME_FAILED:
        return "failed to convert platform local time";
    case cypher_system_error_code_t::ERR_INTERNAL_ERROR:
        return "internal sys error";
    default:
        return "unknown sys error";
    }
}

constexpr inline common::cypher_common_error_t CypherSystem_ErrorCode( cypher_system_error_code_t error ) {
	return common::CypherCommon_ErrorMake( common::cypher_common_domain_t::COM_DOMAIN_SYS, static_cast<common::com_u16>( error ) );
}

}       // namespace cypher::engine::sys

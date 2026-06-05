#pragma once

#include "CypherEngine/CypherCommon/CypherCommon_Error.h"

namespace cypher::engine::sys {

/*
================
System Error Codes
================
*/
enum class error_code_t : common::u8 {
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
constexpr inline const char *CypherSystem_ErrorName( const error_code_t error ) {
    switch ( error ) {
    case error_code_t::OK:
        return "OK";
    case error_code_t::ERR_NOT_INIT:
        return "ERR_NOT_INIT";
    case error_code_t::ERR_IS_INIT:
        return "ERR_IS_INIT";
    case error_code_t::ERR_INVALID_ARGUMENT:
        return "ERR_INVALID_ARGUMENT";
    case error_code_t::ERR_INVALID_PATH:
        return "ERR_INVALID_PATH";
    case error_code_t::ERR_UNSUPPORTED_PLATFORM:
        return "ERR_UNSUPPORTED_PLATFORM";
    case error_code_t::ERR_UNSUPPORTED_COMPILER:
        return "ERR_UNSUPPORTED_COMPILER";
    case error_code_t::ERR_PATH_QUERY_FAILED:
        return "ERR_PATH_QUERY_FAILED";
    case error_code_t::ERR_PATH_TOO_LONG:
        return "ERR_PATH_TOO_LONG";
    case error_code_t::ERR_DIRECTORY_CREATE_FAILED:
        return "ERR_DIRECTORY_CREATE_FAILED";
    case error_code_t::ERR_TIME_UNAVAILABLE:
        return "ERR_TIME_UNAVAILABLE";
    case error_code_t::ERR_LOCALTIME_FAILED:
        return "ERR_LOCALTIME_FAILED";
    case error_code_t::ERR_INTERNAL_ERROR:
        return "ERR_INTERNAL_ERROR";
    default:
        return "ERR_UNKNOWN";
    }
}

constexpr inline const char *CypherSystem_ErrorDesc( const error_code_t error ) {
    switch ( error ) {
    case error_code_t::OK:
        return "operation completed successfully";
    case error_code_t::ERR_NOT_INIT:
        return "sys subsystem is not initialized";
    case error_code_t::ERR_IS_INIT:
        return "sys subsystem is already initialized";
    case error_code_t::ERR_INVALID_ARGUMENT:
        return "invalid sys argument";
    case error_code_t::ERR_INVALID_PATH:
        return "invalid sys path";
    case error_code_t::ERR_UNSUPPORTED_PLATFORM:
        return "unsupported platform";
    case error_code_t::ERR_UNSUPPORTED_COMPILER:
        return "unsupported compiler";
    case error_code_t::ERR_PATH_QUERY_FAILED:
        return "failed to query platform path";
    case error_code_t::ERR_PATH_TOO_LONG:
        return "platform path is too long";
    case error_code_t::ERR_DIRECTORY_CREATE_FAILED:
        return "failed to create platform directory";
    case error_code_t::ERR_TIME_UNAVAILABLE:
        return "platform time source is unavailable";
    case error_code_t::ERR_LOCALTIME_FAILED:
        return "failed to convert platform local time";
    case error_code_t::ERR_INTERNAL_ERROR:
        return "internal sys error";
    default:
        return "unknown sys error";
    }
}

constexpr inline common::error_t CypherSystem_ErrorCode( error_code_t error ) {
	return common::CypherCommon_ErrorMake( common::domain_t::COM_DOMAIN_SYS, static_cast<common::com_u16>( error ) );
}

}       // namespace cypher::engine::sys

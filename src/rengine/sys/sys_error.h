#pragma once

#include "rengine/rcommon/com_error.h"

namespace reap::rengine::sys {

/*
================
System Error Codes
================
*/
enum class sys_error_code_t : rcommon::u8 {
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
constexpr inline const char *Sys_ErrorName( const sys_error_code_t error ) {
    switch ( error ) {
    case sys_error_code_t::OK:
        return "OK";
    case sys_error_code_t::ERR_NOT_INIT:
        return "ERR_NOT_INIT";
    case sys_error_code_t::ERR_IS_INIT:
        return "ERR_IS_INIT";
    case sys_error_code_t::ERR_INVALID_ARGUMENT:
        return "ERR_INVALID_ARGUMENT";
    case sys_error_code_t::ERR_INVALID_PATH:
        return "ERR_INVALID_PATH";
    case sys_error_code_t::ERR_UNSUPPORTED_PLATFORM:
        return "ERR_UNSUPPORTED_PLATFORM";
    case sys_error_code_t::ERR_UNSUPPORTED_COMPILER:
        return "ERR_UNSUPPORTED_COMPILER";
    case sys_error_code_t::ERR_PATH_QUERY_FAILED:
        return "ERR_PATH_QUERY_FAILED";
    case sys_error_code_t::ERR_PATH_TOO_LONG:
        return "ERR_PATH_TOO_LONG";
    case sys_error_code_t::ERR_DIRECTORY_CREATE_FAILED:
        return "ERR_DIRECTORY_CREATE_FAILED";
    case sys_error_code_t::ERR_TIME_UNAVAILABLE:
        return "ERR_TIME_UNAVAILABLE";
    case sys_error_code_t::ERR_LOCALTIME_FAILED:
        return "ERR_LOCALTIME_FAILED";
    case sys_error_code_t::ERR_INTERNAL_ERROR:
        return "ERR_INTERNAL_ERROR";
    default:
        return "ERR_UNKNOWN";
    }
}

constexpr inline const char *Sys_ErrorDesc( const sys_error_code_t error ) {
    switch ( error ) {
    case sys_error_code_t::OK:
        return "operation completed successfully";
    case sys_error_code_t::ERR_NOT_INIT:
        return "sys subsystem is not initialized";
    case sys_error_code_t::ERR_IS_INIT:
        return "sys subsystem is already initialized";
    case sys_error_code_t::ERR_INVALID_ARGUMENT:
        return "invalid sys argument";
    case sys_error_code_t::ERR_INVALID_PATH:
        return "invalid sys path";
    case sys_error_code_t::ERR_UNSUPPORTED_PLATFORM:
        return "unsupported platform";
    case sys_error_code_t::ERR_UNSUPPORTED_COMPILER:
        return "unsupported compiler";
    case sys_error_code_t::ERR_PATH_QUERY_FAILED:
        return "failed to query platform path";
    case sys_error_code_t::ERR_PATH_TOO_LONG:
        return "platform path is too long";
    case sys_error_code_t::ERR_DIRECTORY_CREATE_FAILED:
        return "failed to create platform directory";
    case sys_error_code_t::ERR_TIME_UNAVAILABLE:
        return "platform time source is unavailable";
    case sys_error_code_t::ERR_LOCALTIME_FAILED:
        return "failed to convert platform local time";
    case sys_error_code_t::ERR_INTERNAL_ERROR:
        return "internal sys error";
    default:
        return "unknown sys error";
    }
}

constexpr inline rcommon::com_error_t Sys_ErrorCode( sys_error_code_t error ) {
	return rcommon::Com_ErrorMake( rcommon::com_domain_t::COM_DOMAIN_SYS, static_cast<rcommon::com_u16>( error ) );
}

}       // namespace reap::rengine::sys

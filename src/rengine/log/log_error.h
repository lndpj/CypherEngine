#pragma once

#include "rengine/rcommon/com_error.h"

namespace reap::rengine::log
{

/*
================
Log Error Codes
================
*/
enum class log_error_code_t : rcommon::u8 {
      OK = 0,

      ERR_NOT_INIT,
      ERR_IS_INIT,
      ERR_INVALID_CONFIG,
      ERR_FILE_OPEN_FAILED,
      ERR_FILE_WRITE_FAILED,
      ERR_INVALID_CHANNEL,
      ERR_INVALID_LEVEL
};

/*
================
Log Error Helpers
================
*/
constexpr inline const char *Log_ErrorName( const log_error_code_t error ) {
    switch ( error ) {
    case log_error_code_t::OK:
        return "OK";
    case log_error_code_t::ERR_NOT_INIT:
        return "ERR_NOT_INIT";
    case log_error_code_t::ERR_IS_INIT:
        return "ERR_IS_INIT";
    case log_error_code_t::ERR_INVALID_CONFIG:
        return "ERR_INVALID_CONFIG";
    case log_error_code_t::ERR_FILE_OPEN_FAILED:
        return "ERR_FILE_OPEN_FAILED";
    case log_error_code_t::ERR_FILE_WRITE_FAILED:
        return "ERR_FILE_WRITE_FAILED";
    case log_error_code_t::ERR_INVALID_CHANNEL:
        return "ERR_INVALID_CHANNEL";
    case log_error_code_t::ERR_INVALID_LEVEL:
        return "ERR_INVALID_LEVEL";
    default:
        return "ERR_UNKNOWN";
    }
}

constexpr inline const char *Log_ErrorDesc( const log_error_code_t error ) {
    switch ( error ) {
    case log_error_code_t::OK:
        return "operation completed successfully";
    case log_error_code_t::ERR_NOT_INIT:
        return "log subsystem is not initialized";
    case log_error_code_t::ERR_IS_INIT:
        return "log subsystem is already initialized";
    case log_error_code_t::ERR_INVALID_CONFIG:
        return "invalid log configuration";
    case log_error_code_t::ERR_FILE_OPEN_FAILED:
        return "log subsystem file open failure";
    case log_error_code_t::ERR_FILE_WRITE_FAILED:
        return "log subsystem file write failure";
    case log_error_code_t::ERR_INVALID_CHANNEL:
        return "log subsystem invalid channel";
    case log_error_code_t::ERR_INVALID_LEVEL:
        return "log subsystem invalid level";
    default:
        return "unknown log error";
    }
}

constexpr inline rcommon::com_error_t Log_ErrorCode( log_error_code_t error ) {
    return rcommon::Com_ErrorMake( rcommon::com_domain_t::COM_DOMAIN_LOG, static_cast<rcommon::com_u16>( error ) );
}

}

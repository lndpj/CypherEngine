#pragma once

#include "CypherEngine/CypherCommon/CypherCommon_Error.h"

namespace cypher::engine::log
{

/*
================
Log Error Codes
================
*/
enum class error_code_t : common::u8 {
      OK = 0,

      ERR_NOT_INIT,
      ERR_IS_INIT,
      ERR_INVALID_CONFIG,
      ERR_FILE_OPEN_FAILED,
      ERR_FILE_WRITE_FAILED,
      ERR_FORMAT_FAILED,
      ERR_INVALID_CHANNEL,
      ERR_INVALID_LEVEL
};

/*
================
Log Error Helpers
================
*/
constexpr inline const char *CypherLog_ErrorName( const error_code_t error ) {
    switch ( error ) {
    case error_code_t::OK:
        return "OK";
    case error_code_t::ERR_NOT_INIT:
        return "ERR_NOT_INIT";
    case error_code_t::ERR_IS_INIT:
        return "ERR_IS_INIT";
    case error_code_t::ERR_INVALID_CONFIG:
        return "ERR_INVALID_CONFIG";
    case error_code_t::ERR_FILE_OPEN_FAILED:
        return "ERR_FILE_OPEN_FAILED";
    case error_code_t::ERR_FILE_WRITE_FAILED:
        return "ERR_FILE_WRITE_FAILED";
    case error_code_t::ERR_FORMAT_FAILED:
        return "ERR_FORMAT_FAILED";
    case error_code_t::ERR_INVALID_CHANNEL:
        return "ERR_INVALID_CHANNEL";
    case error_code_t::ERR_INVALID_LEVEL:
        return "ERR_INVALID_LEVEL";
    default:
        return "ERR_UNKNOWN";
    }
}

constexpr inline const char *CypherLog_ErrorDesc( const error_code_t error ) {
    switch ( error ) {
    case error_code_t::OK:
        return "operation completed successfully";
    case error_code_t::ERR_NOT_INIT:
        return "log subsystem is not initialized";
    case error_code_t::ERR_IS_INIT:
        return "log subsystem is already initialized";
    case error_code_t::ERR_INVALID_CONFIG:
        return "invalid log configuration";
    case error_code_t::ERR_FILE_OPEN_FAILED:
        return "log subsystem file open failure";
    case error_code_t::ERR_FILE_WRITE_FAILED:
        return "log subsystem file write failure";
    case error_code_t::ERR_FORMAT_FAILED:
        return "log subsystem formatting failure";
    case error_code_t::ERR_INVALID_CHANNEL:
        return "log subsystem invalid channel";
    case error_code_t::ERR_INVALID_LEVEL:
        return "log subsystem invalid level";
    default:
        return "unknown log error";
    }
}

constexpr inline common::error_t CypherLog_ErrorCode( error_code_t error ) {
    return common::CypherCommon_ErrorMake( common::domain_t::COM_DOMAIN_LOG, static_cast<common::com_u16>( error ) );
}

}

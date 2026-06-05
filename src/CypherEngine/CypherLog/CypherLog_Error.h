#pragma once

#include "CypherEngine/CypherCommon/CypherCommon_Error.h"

namespace cypher::engine::log
{

/*
================
Log Error Codes
================
*/
enum class cypher_log_error_code_t : common::u8 {
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
constexpr inline const char *CypherLog_ErrorName( const cypher_log_error_code_t error ) {
    switch ( error ) {
    case cypher_log_error_code_t::OK:
        return "OK";
    case cypher_log_error_code_t::ERR_NOT_INIT:
        return "ERR_NOT_INIT";
    case cypher_log_error_code_t::ERR_IS_INIT:
        return "ERR_IS_INIT";
    case cypher_log_error_code_t::ERR_INVALID_CONFIG:
        return "ERR_INVALID_CONFIG";
    case cypher_log_error_code_t::ERR_FILE_OPEN_FAILED:
        return "ERR_FILE_OPEN_FAILED";
    case cypher_log_error_code_t::ERR_FILE_WRITE_FAILED:
        return "ERR_FILE_WRITE_FAILED";
    case cypher_log_error_code_t::ERR_INVALID_CHANNEL:
        return "ERR_INVALID_CHANNEL";
    case cypher_log_error_code_t::ERR_INVALID_LEVEL:
        return "ERR_INVALID_LEVEL";
    default:
        return "ERR_UNKNOWN";
    }
}

constexpr inline const char *CypherLog_ErrorDesc( const cypher_log_error_code_t error ) {
    switch ( error ) {
    case cypher_log_error_code_t::OK:
        return "operation completed successfully";
    case cypher_log_error_code_t::ERR_NOT_INIT:
        return "log subsystem is not initialized";
    case cypher_log_error_code_t::ERR_IS_INIT:
        return "log subsystem is already initialized";
    case cypher_log_error_code_t::ERR_INVALID_CONFIG:
        return "invalid log configuration";
    case cypher_log_error_code_t::ERR_FILE_OPEN_FAILED:
        return "log subsystem file open failure";
    case cypher_log_error_code_t::ERR_FILE_WRITE_FAILED:
        return "log subsystem file write failure";
    case cypher_log_error_code_t::ERR_INVALID_CHANNEL:
        return "log subsystem invalid channel";
    case cypher_log_error_code_t::ERR_INVALID_LEVEL:
        return "log subsystem invalid level";
    default:
        return "unknown log error";
    }
}

constexpr inline common::cypher_common_error_t CypherLog_ErrorCode( cypher_log_error_code_t error ) {
    return common::CypherCommon_ErrorMake( common::cypher_common_domain_t::COM_DOMAIN_LOG, static_cast<common::com_u16>( error ) );
}

}

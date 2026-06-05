#pragma once

#include "CypherEngine/CypherCommon/CypherCommon_Error.h"

namespace cypher::engine::host
{

/*
================
Host Error Codes
================
*/
enum class error_code_t : common::u8 {
    OK = 0,

    ERR_NOT_INIT,
    ERR_IS_INIT,
    ERR_INITIALIZING,
};

/*
================
Host Error Helpers
================
*/
constexpr inline const char *CypherHost_ErrorName( const error_code_t error ) {
    switch ( error ) {
    case error_code_t::OK:
        return "OK";
    case error_code_t::ERR_NOT_INIT:
        return "ERR_NOT_INIT";
    case error_code_t::ERR_IS_INIT:
        return "ERR_IS_INIT";
    case error_code_t::ERR_INITIALIZING:
        return "ERR_INITIALIZING";
    default:
        return "ERR_UNKNOWN";
    }
}

constexpr inline const char *CypherHost_ErrorDesc( const error_code_t error ) {
    switch ( error ) {
    case error_code_t::OK:
        return "operation completed successfully";
    case error_code_t::ERR_NOT_INIT:
        return "host subsystem is not initialized";
    case error_code_t::ERR_IS_INIT:
        return "host subsystem is already initialized";
    case error_code_t::ERR_INITIALIZING:
        return "host subsystem failed while initializing";
    default:
        return "unknown host error";
    }
}

constexpr inline common::error_t CypherHost_ErrorCode( error_code_t error ) {
    return common::CypherCommon_ErrorMake( common::domain_t::COM_DOMAIN_HOST, static_cast<common::com_u16>( error ) );
}

}

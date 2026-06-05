#pragma once

#include "CypherEngine/CypherCommon/CypherCommon_Error.h"

namespace cypher::engine::host
{

/*
================
Host Error Codes
================
*/
enum class cypher_host_error_code_t : common::u8 {
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
constexpr inline const char *CypherHost_ErrorName( const cypher_host_error_code_t error ) {
    switch ( error ) {
    case cypher_host_error_code_t::OK:
        return "OK";
    case cypher_host_error_code_t::ERR_NOT_INIT:
        return "ERR_NOT_INIT";
    case cypher_host_error_code_t::ERR_IS_INIT:
        return "ERR_IS_INIT";
    case cypher_host_error_code_t::ERR_INITIALIZING:
        return "ERR_INITIALIZING";
    default:
        return "ERR_UNKNOWN";
    }
}

constexpr inline const char *CypherHost_ErrorDesc( const cypher_host_error_code_t error ) {
    switch ( error ) {
    case cypher_host_error_code_t::OK:
        return "operation completed successfully";
    case cypher_host_error_code_t::ERR_NOT_INIT:
        return "host subsystem is not initialized";
    case cypher_host_error_code_t::ERR_IS_INIT:
        return "host subsystem is already initialized";
    case cypher_host_error_code_t::ERR_INITIALIZING:
        return "host subsystem failed while initializing";
    default:
        return "unknown host error";
    }
}

constexpr inline common::cypher_common_error_t CypherHost_ErrorCode( cypher_host_error_code_t error ) {
    return common::CypherCommon_ErrorMake( common::cypher_common_domain_t::COM_DOMAIN_HOST, static_cast<common::com_u16>( error ) );
}

}

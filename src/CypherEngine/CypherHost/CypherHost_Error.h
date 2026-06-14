#ifndef CYPHER_ENGINE_HOST_ERROR_H
#define CYPHER_ENGINE_HOST_ERROR_H

#pragma once

#include "CypherEngine/CypherCommon/CypherCommon_Error.h"

namespace cypher::engine::host
{

/*
================
Host Error Codes
================
*/
enum class host_error_t : common::u8 {
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
constexpr inline const char *CypherHost_ErrorName( const host_error_t error ) {
    switch ( error ) {
    case host_error_t::OK:
        return "OK";
    case host_error_t::ERR_NOT_INIT:
        return "ERR_NOT_INIT";
    case host_error_t::ERR_IS_INIT:
        return "ERR_IS_INIT";
    case host_error_t::ERR_INITIALIZING:
        return "ERR_INITIALIZING";
    default:
        return "ERR_UNKNOWN";
    }
}

constexpr inline const char *CypherHost_ErrorDesc( const host_error_t error ) {
    switch ( error ) {
    case host_error_t::OK:
        return "operation completed successfully";
    case host_error_t::ERR_NOT_INIT:
        return "host subsystem is not initialized";
    case host_error_t::ERR_IS_INIT:
        return "host subsystem is already initialized";
    case host_error_t::ERR_INITIALIZING:
        return "host subsystem failed while initializing";
    default:
        return "unknown host error";
    }
}

constexpr inline common::error_t CypherHost_ErrorCode( host_error_t error ) {
    return common::CypherCommon_ErrorMake( common::domain_t::COM_DOMAIN_HOST, static_cast<common::com_u16>( error ) );
}

}

#endif // CYPHER_ENGINE_HOST_ERROR_H

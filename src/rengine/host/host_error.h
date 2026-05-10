#pragma once

#include "rengine/rcommon/com_error.h"

namespace reap::rengine::host
{

/*
================
Host Error Codes
================
*/
enum class host_error_code_t : rcommon::u8 {
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
constexpr inline const char *Host_ErrorName( const host_error_code_t error ) {
    switch ( error ) {
    case host_error_code_t::OK:
        return "OK";
    case host_error_code_t::ERR_NOT_INIT:
        return "ERR_NOT_INIT";
    case host_error_code_t::ERR_IS_INIT:
        return "ERR_IS_INIT";
    case host_error_code_t::ERR_INITIALIZING:
        return "ERR_INITIALIZING";
    default:
        return "ERR_UNKNOWN";
    }
}

constexpr inline const char *Host_ErrorDesc( const host_error_code_t error ) {
    switch ( error ) {
    case host_error_code_t::OK:
        return "operation completed successfully";
    case host_error_code_t::ERR_NOT_INIT:
        return "host subsystem is not initialized";
    case host_error_code_t::ERR_IS_INIT:
        return "host subsystem is already initialized";
    case host_error_code_t::ERR_INITIALIZING:
        return "host subsystem failed while initializing";
    default:
        return "unknown host error";
    }
}

constexpr inline rcommon::com_error_t Host_ErrorCode( host_error_code_t error ) {
    return rcommon::Com_ErrorMake( rcommon::com_domain_t::COM_DOMAIN_HOST, static_cast<rcommon::com_u16>( error ) );
}

}

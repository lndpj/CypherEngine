#pragma once

#include "CypherEngine/CypherCommon/CypherCommon.h"
#include "CypherEngine/CypherCommon/CypherCommon_Error.h"

namespace cypher::engine::memory
{

enum class error_code_t : common::u8 {
    OK = 0,
    
    ERR_INVALID_ARGUMENT,
    ERR_OUT_OF_MEMORY,
    ERR_MEMORY_ALLOCATION,
    ERR_ALREADY_INITIALIZED,
    ERR_NOT_INITIALIZED,
    ERR_INVALID_ALIGNMENT,
    ERR_INVALID_CAPACITY,
    ERR_INVALID_MARKER,
    ERR_BUFFER_TOO_SMALL,
    ERR_EXTERNAL_BUFFER_REQUIRED
};

constexpr inline const char *CypherMemory_ErrorName( const error_code_t error )
{
    switch ( error ) {
    case error_code_t::OK:
        return "OK";
    case error_code_t::ERR_INVALID_ARGUMENT:
        return "ERR_INVALID_ARGUMENT";
    case error_code_t::ERR_OUT_OF_MEMORY:
        return "ERR_OUT_OF_MEMORY";
    case error_code_t::ERR_MEMORY_ALLOCATION:
        return "ERR_MEMORY_ALLOCATION";
    case error_code_t::ERR_ALREADY_INITIALIZED:
        return "ERR_ALREADY_INITIALIZED";
    case error_code_t::ERR_NOT_INITIALIZED:
        return "ERR_NOT_INITIALIZED";
    case error_code_t::ERR_INVALID_ALIGNMENT:
        return "ERR_INVALID_ALIGNMENT";
    case error_code_t::ERR_INVALID_CAPACITY:
        return "ERR_INVALID_CAPACITY";
    case error_code_t::ERR_INVALID_MARKER:
        return "ERR_INVALID_MARKER";
    case error_code_t::ERR_EXTERNAL_BUFFER_REQUIRED:
        return "ERR_EXTERNAL_BUFFER_REQUIRED";
    default:
        return "ERR_UNKNOWN";
    }
}

constexpr inline const char *CypherMemory_ErrorDesc( const error_code_t error )
{
    switch ( error ) {
    case error_code_t::OK:
        return "operation completed successfully";
    case error_code_t::ERR_INVALID_ARGUMENT:
        return "invalid argument passed to memory subsystem";
    case error_code_t::ERR_OUT_OF_MEMORY:
        return "arena or allocator does not have enough remaining memory";
    case error_code_t::ERR_MEMORY_ALLOCATION:
        return "memory allocation failure";
    case error_code_t::ERR_ALREADY_INITIALIZED:
        return "memory object is already initialized";
    case error_code_t::ERR_NOT_INITIALIZED:
        return "memory object is not initialized";
    case error_code_t::ERR_INVALID_ALIGNMENT:
        return "requested alignment is invalid";
    case error_code_t::ERR_INVALID_CAPACITY:
        return "requested allocator capacity is invalid";
    case error_code_t::ERR_INVALID_MARKER:
        return "arena marker is invalid for this arena state";
    case error_code_t::ERR_BUFFER_TOO_SMALL:
        return "provided buffer is too small";
    case error_code_t::ERR_EXTERNAL_BUFFER_REQUIRED:
        return "external memory buffer is required";
    default:
        return "unknown memory subsystem error";
    }
}

constexpr inline common::error_t CypherMemory_ErrorCode( const error_code_t code ) 
{
    return common::CypherCommon_ErrorMake( common::domain_t::COM_DOMAIN_MEMORY, static_cast<common::u16>( code ) );    
}

}       // namespace cypher::engine::memory

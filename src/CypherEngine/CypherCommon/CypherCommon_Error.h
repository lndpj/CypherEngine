#pragma once

#include "CypherEngine/CypherCommon/CypherCommon.h"

namespace cypher::engine::common {

using error_t = com_u32;

/*
================
Common Error Encoding

Errors are packed as high 16 bits domain, low 16 bits subsystem-local code.
================
*/
enum class domain_t : u16 {
	COM_DOMAIN_COMMON = 0,
	COM_DOMAIN_RENDER,
	COM_DOMAIN_LOG,
	COM_DOMAIN_HOST,
	COM_DOMAIN_SYS,
	COM_DOMAIN_AUDIO,
	COM_DOMAIN_FS,
	COM_DOMAIN_NET,
	COM_DOMAIN_GAME,
	COM_DOMAIN_CMD,
	COM_DOMAIN_CVAR,
	COM_DOMAIN_CFG,
    COM_DOMAIN_MEMORY
};

enum class error_code_t : com_u8 {
	OK = 0,
    
	ERR_FAILED,
	ERR_INVALID_ARGUMENT,
	ERR_INVALID_STATE,
	ERR_INVALID_OPERATION,
	ERR_NOT_INIT,
	ERR_IS_INIT,
	ERR_OUT_OF_MEMORY,
	ERR_NOT_FOUND,
	ERR_UNSUPPORTED,
	ERR_IO_ERROR,
	ERR_INTERNAL_ERROR
};

/*
================
Common Error Helpers
================
*/
constexpr bool CypherCommon_ErrorOk( const error_code_t code ) {
	return code == error_code_t::OK;
}

constexpr bool CypherCommon_ErrorFailed( const error_code_t code ) {
	return code != error_code_t::OK;
}

constexpr inline const char *CypherCommon_ErrorName( const error_code_t code ) {
	switch ( code ) {
	case error_code_t::OK:
		return "OK";
	case error_code_t::ERR_FAILED:
		return "COM_FAILED";
	case error_code_t::ERR_INVALID_ARGUMENT:
		return "COM_INVALID_ARGUMENT";
	case error_code_t::ERR_INVALID_STATE:
		return "COM_INVALID_STATE";
	case error_code_t::ERR_INVALID_OPERATION:
		return "COM_INVALID_OPERATION";
	case error_code_t::ERR_NOT_INIT:
		return "COM_NOT_INITIALIZED";
	case error_code_t::ERR_IS_INIT:
		return "COM_ALREADY_INITIALIZED";
	case error_code_t::ERR_OUT_OF_MEMORY:
		return "COM_OUT_OF_MEMORY";
	case error_code_t::ERR_NOT_FOUND:
		return "NOT_FOUND";
	case error_code_t::ERR_UNSUPPORTED:
		return "COM_UNSUPPORTED";
	case error_code_t::ERR_IO_ERROR:
		return "IO_ERROR";
	case error_code_t::ERR_INTERNAL_ERROR:
		return "COM_INTERNAL_ERROR";
	default:
		return "COM_UNKNOWN_ERROR";
	}
}

constexpr inline const char *CypherCommon_DomainName( const domain_t domain ) {
	switch ( domain ) {
	case domain_t::COM_DOMAIN_HOST:
		return "HOST";
	case domain_t::COM_DOMAIN_GAME:
		return "GAME";
	case domain_t::COM_DOMAIN_SYS:
		return "SYS";
	case domain_t::COM_DOMAIN_AUDIO:
		return "AUDIO";
	case domain_t::COM_DOMAIN_COMMON:
		return "COMMON";
	case domain_t::COM_DOMAIN_LOG:
		return "LOG";
	case domain_t::COM_DOMAIN_RENDER:
		return "RENDER";
	case domain_t::COM_DOMAIN_FS:
		return "FS";
	case domain_t::COM_DOMAIN_NET:
		return "NET";
	case domain_t::COM_DOMAIN_CMD:
		return "CMD";
    case domain_t::COM_DOMAIN_CVAR:
        return "CVAR";
    case domain_t::COM_DOMAIN_CFG:
        return "CFG";
    case domain_t::COM_DOMAIN_MEMORY:
        return "MEMORY";
	default:
		return "UNKNOWN";
	}
}

constexpr inline error_t CypherCommon_ErrorMake( const domain_t domain, const com_u16 local_error_code ) {
	return ( static_cast<error_t>( domain ) << 16u ) | static_cast<error_t>( local_error_code );
}

constexpr inline domain_t CypherCommon_ErrorDomain( const error_t error ) {
	return static_cast<domain_t>( ( error >> 16u ) & 0xFFFFu );
}

constexpr inline com_u16 CypherCommon_ErrorCode( const error_t error ) {
	return static_cast<com_u16>( error & 0xFFFFu );
}

}

#pragma once

#include "CypherEngine/CypherCommon/CypherCommon.h"

namespace cypher::engine::common {

using cypher_common_error_t = com_u32;

/*
================
Common Error Encoding

Errors are packed as high 16 bits domain, low 16 bits subsystem-local code.
================
*/
enum class cypher_common_domain_t : u16 {
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
	COM_DOMAIN_CFG
};

enum class cypher_common_error_code_t : com_u8 {
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
constexpr bool CypherCommon_ErrorOk( const cypher_common_error_code_t code ) {
	return code == cypher_common_error_code_t::OK;
}

constexpr bool CypherCommon_ErrorFailed( const cypher_common_error_code_t code ) {
	return code != cypher_common_error_code_t::OK;
}

constexpr inline const char *CypherCommon_ErrorName( const cypher_common_error_code_t code ) {
	switch ( code ) {
	case cypher_common_error_code_t::OK:
		return "OK";
	case cypher_common_error_code_t::ERR_FAILED:
		return "COM_FAILED";
	case cypher_common_error_code_t::ERR_INVALID_ARGUMENT:
		return "COM_INVALID_ARGUMENT";
	case cypher_common_error_code_t::ERR_INVALID_STATE:
		return "COM_INVALID_STATE";
	case cypher_common_error_code_t::ERR_INVALID_OPERATION:
		return "COM_INVALID_OPERATION";
	case cypher_common_error_code_t::ERR_NOT_INIT:
		return "COM_NOT_INITIALIZED";
	case cypher_common_error_code_t::ERR_IS_INIT:
		return "COM_ALREADY_INITIALIZED";
	case cypher_common_error_code_t::ERR_OUT_OF_MEMORY:
		return "COM_OUT_OF_MEMORY";
	case cypher_common_error_code_t::ERR_NOT_FOUND:
		return "NOT_FOUND";
	case cypher_common_error_code_t::ERR_UNSUPPORTED:
		return "COM_UNSUPPORTED";
	case cypher_common_error_code_t::ERR_IO_ERROR:
		return "IO_ERROR";
	case cypher_common_error_code_t::ERR_INTERNAL_ERROR:
		return "COM_INTERNAL_ERROR";
	default:
		return "COM_UNKNOWN_ERROR";
	}
}

constexpr inline const char *CypherCommon_DomainName( const cypher_common_domain_t domain ) {
	switch ( domain ) {
	case cypher_common_domain_t::COM_DOMAIN_HOST:
		return "HOST";
	case cypher_common_domain_t::COM_DOMAIN_GAME:
		return "GAME";
	case cypher_common_domain_t::COM_DOMAIN_SYS:
		return "SYS";
	case cypher_common_domain_t::COM_DOMAIN_AUDIO:
		return "AUDIO";
	case cypher_common_domain_t::COM_DOMAIN_COMMON:
		return "COMMON";
	case cypher_common_domain_t::COM_DOMAIN_LOG:
		return "LOG";
	case cypher_common_domain_t::COM_DOMAIN_RENDER:
		return "RENDER";
	case cypher_common_domain_t::COM_DOMAIN_FS:
		return "FS";
	case cypher_common_domain_t::COM_DOMAIN_NET:
		return "NET";
	case cypher_common_domain_t::COM_DOMAIN_CMD:
		return "CMD";
	case cypher_common_domain_t::COM_DOMAIN_CVAR:
		return "CVAR";
	default:
		return "UNKNOWN";
	}
}

constexpr inline cypher_common_error_t CypherCommon_ErrorMake( const cypher_common_domain_t domain, const com_u16 local_error_code ) {
	return ( static_cast<cypher_common_error_t>( domain ) << 16u ) | static_cast<cypher_common_error_t>( local_error_code );
}

constexpr inline cypher_common_domain_t CypherCommon_ErrorDomain( const cypher_common_error_t error ) {
	return static_cast<cypher_common_domain_t>( ( error >> 16u ) & 0xFFFFu );
}

constexpr inline com_u16 CypherCommon_ErrorCode( const cypher_common_error_t error ) {
	return static_cast<com_u16>( error & 0xFFFFu );
}

}

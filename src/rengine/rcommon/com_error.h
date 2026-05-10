#pragma once

#include "rengine/rcommon/com_main.h"

namespace reap::rengine::rcommon {

using com_error_t = com_u32;

/*
================
Common Error Encoding

Errors are packed as high 16 bits domain, low 16 bits subsystem-local code.
================
*/
enum class com_domain_t : u16 {
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

enum class com_error_code_t : com_u8 {
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
constexpr bool Com_ErrorOk( const com_error_code_t code ) {
	return code == com_error_code_t::OK;
}

constexpr bool Com_ErrorFailed( const com_error_code_t code ) {
	return code != com_error_code_t::OK;
}

constexpr inline const char *Com_ErrorName( const com_error_code_t code ) {
	switch ( code ) {
	case com_error_code_t::OK:
		return "OK";
	case com_error_code_t::ERR_FAILED:
		return "COM_FAILED";
	case com_error_code_t::ERR_INVALID_ARGUMENT:
		return "COM_INVALID_ARGUMENT";
	case com_error_code_t::ERR_INVALID_STATE:
		return "COM_INVALID_STATE";
	case com_error_code_t::ERR_INVALID_OPERATION:
		return "COM_INVALID_OPERATION";
	case com_error_code_t::ERR_NOT_INIT:
		return "COM_NOT_INITIALIZED";
	case com_error_code_t::ERR_IS_INIT:
		return "COM_ALREADY_INITIALIZED";
	case com_error_code_t::ERR_OUT_OF_MEMORY:
		return "COM_OUT_OF_MEMORY";
	case com_error_code_t::ERR_NOT_FOUND:
		return "NOT_FOUND";
	case com_error_code_t::ERR_UNSUPPORTED:
		return "COM_UNSUPPORTED";
	case com_error_code_t::ERR_IO_ERROR:
		return "IO_ERROR";
	case com_error_code_t::ERR_INTERNAL_ERROR:
		return "COM_INTERNAL_ERROR";
	default:
		return "COM_UNKNOWN_ERROR";
	}
}

constexpr inline const char *Com_DomainName( const com_domain_t domain ) {
	switch ( domain ) {
	case com_domain_t::COM_DOMAIN_HOST:
		return "HOST";
	case com_domain_t::COM_DOMAIN_GAME:
		return "GAME";
	case com_domain_t::COM_DOMAIN_SYS:
		return "SYS";
	case com_domain_t::COM_DOMAIN_AUDIO:
		return "AUDIO";
	case com_domain_t::COM_DOMAIN_COMMON:
		return "COMMON";
	case com_domain_t::COM_DOMAIN_LOG:
		return "LOG";
	case com_domain_t::COM_DOMAIN_RENDER:
		return "RENDER";
	case com_domain_t::COM_DOMAIN_FS:
		return "FS";
	case com_domain_t::COM_DOMAIN_NET:
		return "NET";
	case com_domain_t::COM_DOMAIN_CMD:
		return "CMD";
	case com_domain_t::COM_DOMAIN_CVAR:
		return "CVAR";
	default:
		return "UNKNOWN";
	}
}

constexpr inline com_error_t Com_ErrorMake( const com_domain_t domain, const com_u16 local_error_code ) {
	return ( static_cast<com_error_t>( domain ) << 16u ) | static_cast<com_error_t>( local_error_code );
}

constexpr inline com_domain_t Com_ErrorDomain( const com_error_t error ) {
	return static_cast<com_domain_t>( ( error >> 16u ) & 0xFFFFu );
}

constexpr inline com_u16 Com_ErrorCode( const com_error_t error ) {
	return static_cast<com_u16>( error & 0xFFFFu );
}

}

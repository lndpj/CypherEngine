#pragma once

#include "rengine/rcommon/com_main.h"
#include "rengine/rcommon/com_error.h"

namespace reap::rengine::cvar {

/*
================
Cvar Error Codes
================
*/
enum class cvar_error_code_t : rcommon::u8 {
	OK = 0,

	ERR_NOT_INIT,
	ERR_IS_INIT,
	ERR_INVALID_CVAR,
	ERR_INVALID_DEFAULT_VALUE,
	ERR_INVALID_FLAG,
	ERR_CVAR_ALREADY_EXISTS,
	ERR_CVAR_NOT_FOUND,
	ERR_REGISTRY_FULL,
	ERR_READONLY,
	ERR_CHEAT_PROTECTED
};

/*
================
Cvar Error Helpers
================
*/
constexpr inline const char *Cvar_ErrorName( const cvar_error_code_t error ) {
	switch ( error ) {
	case cvar_error_code_t::OK:
		return "OK";
	case cvar_error_code_t::ERR_NOT_INIT:
		return "ERR_NOT_INIT";
	case cvar_error_code_t::ERR_IS_INIT:
		return "ERR_IS_INIT";
	case cvar_error_code_t::ERR_INVALID_CVAR:
		return "ERR_INVALID_CVAR";
	case cvar_error_code_t::ERR_CVAR_ALREADY_EXISTS:
		return "ERR_CVAR_ALREADY_EXISTS";
	case cvar_error_code_t::ERR_CVAR_NOT_FOUND:
		return "ERR_CVAR_NOT_FOUND";
	case cvar_error_code_t::ERR_REGISTRY_FULL:
		return "ERR_REGISTRY_FULL";
	case cvar_error_code_t::ERR_INVALID_DEFAULT_VALUE:
		return "ERR_INVALID_DEFAULT_VALUE";
	case cvar_error_code_t::ERR_READONLY:
		return "ERR_READONLY";
	case cvar_error_code_t::ERR_CHEAT_PROTECTED:
		return "ERR_CHEAT_PROTECTED";
	case cvar_error_code_t::ERR_INVALID_FLAG:
		return "ERR_INVALID_FLAG";
	default:
		return "ERR_UNKNOWN";
	}
}

constexpr inline const char *Cvar_ErrorDesc( const cvar_error_code_t error ) {
	switch ( error ) {
	case cvar_error_code_t::OK:
		return "success";
	case cvar_error_code_t::ERR_NOT_INIT:
		return "cvar system is not initialized";
	case cvar_error_code_t::ERR_IS_INIT:
		return "cvar system is already initialized";
	case cvar_error_code_t::ERR_INVALID_CVAR:
		return "invalid cvar name or value was provided";
	case cvar_error_code_t::ERR_CVAR_ALREADY_EXISTS:
		return "cvar is already registered";
	case cvar_error_code_t::ERR_CVAR_NOT_FOUND:
		return "cvar was not found";
	case cvar_error_code_t::ERR_REGISTRY_FULL:
		return "cvar registry is full";
	case cvar_error_code_t::ERR_INVALID_DEFAULT_VALUE:
		return "invalid default cvar value";
	case cvar_error_code_t::ERR_READONLY:
		return "cvar is read-only";
	case cvar_error_code_t::ERR_CHEAT_PROTECTED:
		return "cvar is cheat-protected";
	case cvar_error_code_t::ERR_INVALID_FLAG:
		return "invalid cvar flags were provided";
	default:
		return "unknown cvar error";
	}
}

constexpr inline rcommon::com_error_t Cvar_ErrorCode( cvar_error_code_t error ) {
	return rcommon::Com_ErrorMake( rcommon::com_domain_t::COM_DOMAIN_CVAR, static_cast<rcommon::com_u16>( error ) );
}

}

#pragma once

#include "CypherEngine/CypherCommon/CypherCommon.h"
#include "CypherEngine/CypherCommon/CypherCommon_Error.h"

namespace cypher::engine::cvar {

/*
================
Cvar Error Codes
================
*/
enum class error_code_t : common::u8 {
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
constexpr inline const char *CypherCVar_ErrorName( const error_code_t error ) {
	switch ( error ) {
	case error_code_t::OK:
		return "OK";
	case error_code_t::ERR_NOT_INIT:
		return "ERR_NOT_INIT";
	case error_code_t::ERR_IS_INIT:
		return "ERR_IS_INIT";
	case error_code_t::ERR_INVALID_CVAR:
		return "ERR_INVALID_CVAR";
	case error_code_t::ERR_CVAR_ALREADY_EXISTS:
		return "ERR_CVAR_ALREADY_EXISTS";
	case error_code_t::ERR_CVAR_NOT_FOUND:
		return "ERR_CVAR_NOT_FOUND";
	case error_code_t::ERR_REGISTRY_FULL:
		return "ERR_REGISTRY_FULL";
	case error_code_t::ERR_INVALID_DEFAULT_VALUE:
		return "ERR_INVALID_DEFAULT_VALUE";
	case error_code_t::ERR_READONLY:
		return "ERR_READONLY";
	case error_code_t::ERR_CHEAT_PROTECTED:
		return "ERR_CHEAT_PROTECTED";
	case error_code_t::ERR_INVALID_FLAG:
		return "ERR_INVALID_FLAG";
	default:
		return "ERR_UNKNOWN";
	}
}

constexpr inline const char *CypherCVar_ErrorDesc( const error_code_t error ) {
	switch ( error ) {
	case error_code_t::OK:
		return "success";
	case error_code_t::ERR_NOT_INIT:
		return "cvar system is not initialized";
	case error_code_t::ERR_IS_INIT:
		return "cvar system is already initialized";
	case error_code_t::ERR_INVALID_CVAR:
		return "invalid cvar name or value was provided";
	case error_code_t::ERR_CVAR_ALREADY_EXISTS:
		return "cvar is already registered";
	case error_code_t::ERR_CVAR_NOT_FOUND:
		return "cvar was not found";
	case error_code_t::ERR_REGISTRY_FULL:
		return "cvar registry is full";
	case error_code_t::ERR_INVALID_DEFAULT_VALUE:
		return "invalid default cvar value";
	case error_code_t::ERR_READONLY:
		return "cvar is read-only";
	case error_code_t::ERR_CHEAT_PROTECTED:
		return "cvar is cheat-protected";
	case error_code_t::ERR_INVALID_FLAG:
		return "invalid cvar flags were provided";
	default:
		return "unknown cvar error";
	}
}

constexpr inline common::error_t CypherCVar_ErrorCode( error_code_t error ) {
	return common::CypherCommon_ErrorMake( common::domain_t::COM_DOMAIN_CVAR, static_cast<common::com_u16>( error ) );
}

}

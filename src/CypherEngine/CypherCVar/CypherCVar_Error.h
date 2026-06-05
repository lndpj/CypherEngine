#pragma once

#include "CypherEngine/CypherCommon/CypherCommon.h"
#include "CypherEngine/CypherCommon/CypherCommon_Error.h"

namespace cypher::engine::cvar {

/*
================
Cvar Error Codes
================
*/
enum class cypher_cvar_error_code_t : common::u8 {
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
constexpr inline const char *CypherCVar_ErrorName( const cypher_cvar_error_code_t error ) {
	switch ( error ) {
	case cypher_cvar_error_code_t::OK:
		return "OK";
	case cypher_cvar_error_code_t::ERR_NOT_INIT:
		return "ERR_NOT_INIT";
	case cypher_cvar_error_code_t::ERR_IS_INIT:
		return "ERR_IS_INIT";
	case cypher_cvar_error_code_t::ERR_INVALID_CVAR:
		return "ERR_INVALID_CVAR";
	case cypher_cvar_error_code_t::ERR_CVAR_ALREADY_EXISTS:
		return "ERR_CVAR_ALREADY_EXISTS";
	case cypher_cvar_error_code_t::ERR_CVAR_NOT_FOUND:
		return "ERR_CVAR_NOT_FOUND";
	case cypher_cvar_error_code_t::ERR_REGISTRY_FULL:
		return "ERR_REGISTRY_FULL";
	case cypher_cvar_error_code_t::ERR_INVALID_DEFAULT_VALUE:
		return "ERR_INVALID_DEFAULT_VALUE";
	case cypher_cvar_error_code_t::ERR_READONLY:
		return "ERR_READONLY";
	case cypher_cvar_error_code_t::ERR_CHEAT_PROTECTED:
		return "ERR_CHEAT_PROTECTED";
	case cypher_cvar_error_code_t::ERR_INVALID_FLAG:
		return "ERR_INVALID_FLAG";
	default:
		return "ERR_UNKNOWN";
	}
}

constexpr inline const char *CypherCVar_ErrorDesc( const cypher_cvar_error_code_t error ) {
	switch ( error ) {
	case cypher_cvar_error_code_t::OK:
		return "success";
	case cypher_cvar_error_code_t::ERR_NOT_INIT:
		return "cvar system is not initialized";
	case cypher_cvar_error_code_t::ERR_IS_INIT:
		return "cvar system is already initialized";
	case cypher_cvar_error_code_t::ERR_INVALID_CVAR:
		return "invalid cvar name or value was provided";
	case cypher_cvar_error_code_t::ERR_CVAR_ALREADY_EXISTS:
		return "cvar is already registered";
	case cypher_cvar_error_code_t::ERR_CVAR_NOT_FOUND:
		return "cvar was not found";
	case cypher_cvar_error_code_t::ERR_REGISTRY_FULL:
		return "cvar registry is full";
	case cypher_cvar_error_code_t::ERR_INVALID_DEFAULT_VALUE:
		return "invalid default cvar value";
	case cypher_cvar_error_code_t::ERR_READONLY:
		return "cvar is read-only";
	case cypher_cvar_error_code_t::ERR_CHEAT_PROTECTED:
		return "cvar is cheat-protected";
	case cypher_cvar_error_code_t::ERR_INVALID_FLAG:
		return "invalid cvar flags were provided";
	default:
		return "unknown cvar error";
	}
}

constexpr inline common::cypher_common_error_t CypherCVar_ErrorCode( cypher_cvar_error_code_t error ) {
	return common::CypherCommon_ErrorMake( common::cypher_common_domain_t::COM_DOMAIN_CVAR, static_cast<common::com_u16>( error ) );
}

}

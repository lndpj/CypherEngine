
#pragma once

#include "CypherEngine/CypherCommon/CypherCommon.h"
#include "CypherEngine/CypherCommon/CypherCommon_Error.h"

namespace cypher::engine::cfg {

/*
================
Config Error Codes
================
*/
enum class error_code_t : common::u8 {
	OK = 0,
	ERR_NOT_INIT,
	ERR_IS_INIT,
	ERR_INVALID_PATH,
	ERR_INVALID_LINE,
	ERR_FILE_OPEN_FAILED,
	ERR_PARSE_FAILED,
	ERR_COMMAND_FAILED,
	ERR_IO_ERROR
};

/*
================
Config Error Helpers
================
*/
constexpr inline const char *CypherConfig_ErrorName( const error_code_t error ) {
	switch ( error ) {
	case error_code_t::OK:
		return "OK";
	case error_code_t::ERR_NOT_INIT:
		return "ERR_NOT_INIT";
	case error_code_t::ERR_IS_INIT:
		return "ERR_IS_INIT";
	case error_code_t::ERR_INVALID_PATH:
		return "ERR_INVALID_PATH";
	case error_code_t::ERR_INVALID_LINE:
		return "ERR_INVALID_LINE";
	case error_code_t::ERR_FILE_OPEN_FAILED:
		return "ERR_FILE_OPEN_FAILED";
	case error_code_t::ERR_PARSE_FAILED:
		return "ERR_PARSE_FAILED";
	case error_code_t::ERR_COMMAND_FAILED:
		return "ERR_COMMAND_FAILED";
	case error_code_t::ERR_IO_ERROR:
		return "ERR_IO_ERROR";
	default:
		return "ERR_UNKNOWN";
	}
}

constexpr inline const char *CypherConfig_ErrorDesc( const error_code_t error ) {
	switch ( error ) {
	case error_code_t::OK:
		return "operation completed successfully";
	case error_code_t::ERR_NOT_INIT:
		return "cfg subsystem is not initialized";
	case error_code_t::ERR_IS_INIT:
		return "cfg subsystem is already initialized";
	case error_code_t::ERR_INVALID_PATH:
		return "invalid cfg path";
	case error_code_t::ERR_INVALID_LINE:
		return "invalid cfg line";
	case error_code_t::ERR_FILE_OPEN_FAILED:
		return "unable to open cfg file";
	case error_code_t::ERR_PARSE_FAILED:
		return "one or more cfg lines failed to parse";
	case error_code_t::ERR_COMMAND_FAILED:
		return "command execution failed";
	case error_code_t::ERR_IO_ERROR:
		return "cfg IO error";
	default:
		return "unknown cfg error";
	}
}

constexpr inline common::error_t CypherConfig_ErrorCode( const error_code_t error ) {
	return common::CypherCommon_ErrorMake( common::domain_t::COM_DOMAIN_CFG, static_cast<common::u16>( error ) );
}

}


#pragma once

#include "rengine/rcommon/com_main.h"
#include "rengine/rcommon/com_error.h"

namespace reap::rengine::cfg {

/*
================
Config Error Codes
================
*/
enum class cfg_error_code_t : rcommon::u8 {
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
constexpr inline const char *Cfg_ErrorName( const cfg_error_code_t error ) {
	switch ( error ) {
	case cfg_error_code_t::OK:
		return "OK";
	case cfg_error_code_t::ERR_NOT_INIT:
		return "ERR_NOT_INIT";
	case cfg_error_code_t::ERR_IS_INIT:
		return "ERR_IS_INIT";
	case cfg_error_code_t::ERR_INVALID_PATH:
		return "ERR_INVALID_PATH";
	case cfg_error_code_t::ERR_INVALID_LINE:
		return "ERR_INVALID_LINE";
	case cfg_error_code_t::ERR_FILE_OPEN_FAILED:
		return "ERR_FILE_OPEN_FAILED";
	case cfg_error_code_t::ERR_PARSE_FAILED:
		return "ERR_PARSE_FAILED";
	case cfg_error_code_t::ERR_COMMAND_FAILED:
		return "ERR_COMMAND_FAILED";
	case cfg_error_code_t::ERR_IO_ERROR:
		return "ERR_IO_ERROR";
	default:
		return "ERR_UNKNOWN";
	}
}

constexpr inline const char *Cfg_ErrorDesc( const cfg_error_code_t error ) {
	switch ( error ) {
	case cfg_error_code_t::OK:
		return "operation completed successfully";
	case cfg_error_code_t::ERR_NOT_INIT:
		return "cfg subsystem is not initialized";
	case cfg_error_code_t::ERR_IS_INIT:
		return "cfg subsystem is already initialized";
	case cfg_error_code_t::ERR_INVALID_PATH:
		return "invalid cfg path";
	case cfg_error_code_t::ERR_INVALID_LINE:
		return "invalid cfg line";
	case cfg_error_code_t::ERR_FILE_OPEN_FAILED:
		return "unable to open cfg file";
	case cfg_error_code_t::ERR_PARSE_FAILED:
		return "one or more cfg lines failed to parse";
	case cfg_error_code_t::ERR_COMMAND_FAILED:
		return "command execution failed";
	case cfg_error_code_t::ERR_IO_ERROR:
		return "cfg IO error";
	default:
		return "unknown cfg error";
	}
}

constexpr inline rcommon::com_error_t Cfg_ErrorCode( const cfg_error_code_t error ) {
	return rcommon::Com_ErrorMake( rcommon::com_domain_t::COM_DOMAIN_CFG, static_cast<rcommon::u16>( error ) );
}

}

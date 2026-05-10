#pragma once

#include "rengine/rcommon/com_main.h"

#include <ctime>       // std::time_t timestamps.

namespace reap::rengine::log {

constexpr rcommon::usize REAP_LOG_MESSAGE_MAX = 1024u;

constexpr rcommon::usize REAP_LOG_FILE_PATH_MAX = 512u;

/*
================
Log Types

Severity, output policy, channel and record descriptions for the log system.
================
*/
enum class log_level_t : rcommon::u8 {
	TRACE,
	DEBUG,
	INFO,
	WARNING,
	ERROR,
	FATAL
};

enum class log_file_mode_t : rcommon::u8 {
    TRUNCATE,
    APPEND
};

enum class log_flush_policy_t : rcommon::u8 {
    NEVER,
    ERRORS_AND_ABOVE,
    EVERY_MESSAGE
};

enum class log_source_path_mode_t : rcommon::u8 {
    BASENAME,
    FULL_PATH
};

enum class log_channel_t : rcommon::u8 {
	NONE = 0,
	CORE,
	HOST,
	PLATFORM,
	RENDER,
	PHYSICS,
	NET,
	ECS,
    CFG,
    CMD,
    CVAR,
    FS,
	GAME,
	TOOLS,
	AUDIO,
	ASSETS,
	COUNT
};

/*
================
Log Runtime Records
================
*/
struct log_record_t {
	log_level_t level{ log_level_t::INFO };
	log_channel_t channel{ log_channel_t::CORE };
	const char *file{ "" };
	const char *function{ "" };
	rcommon::com_i32 line{ 0 };
    std::time_t timestamp{};
	char message[REAP_LOG_MESSAGE_MAX]{};
};

struct log_config_t {
	log_level_t min_level{ log_level_t::INFO };
	rcommon::com_u32 channel_mask{ 0xFFFFFFFFu };
	bool console_enabled{ true };
    bool file_enabled{ false };
	bool include_timestamps{ true };
    log_file_mode_t file_mode{ log_file_mode_t::TRUNCATE };
    log_flush_policy_t flush_policy{ log_flush_policy_t::ERRORS_AND_ABOVE };
    log_source_path_mode_t source_path_mode{ log_source_path_mode_t::BASENAME };
    char file_path[REAP_LOG_FILE_PATH_MAX]{ "reap.log" };
};

/*
================
Log Name Helpers
================
*/
constexpr inline const char *Log_LevelName( const log_level_t log_level ) {
	switch ( log_level ) {
        case log_level_t::TRACE:        return "TRACE";
        case log_level_t::DEBUG:        return "DEBUG";
        case log_level_t::INFO:         return "INFO";
        case log_level_t::WARNING:      return "WARNING";
        case log_level_t::ERROR:        return "ERROR";
        case log_level_t::FATAL:        return "FATAL";
        default:                        return "UNKNOWN";
	}
}

constexpr inline const char *Log_ChannelName( const log_channel_t channel ) {
    switch ( channel ) {
        case log_channel_t::CORE:      return "CORE";
        case log_channel_t::HOST:      return "HOST";
        case log_channel_t::PLATFORM:  return "PLATFORM";
        case log_channel_t::RENDER:    return "RENDER";
        case log_channel_t::PHYSICS:   return "PHYSICS";
        case log_channel_t::NET:       return "NET";
        case log_channel_t::ECS:       return "ECS";
        case log_channel_t::CFG:       return "CFG";
        case log_channel_t::CMD:       return "CMD";
        case log_channel_t::CVAR:      return "CVAR";
        case log_channel_t::FS:        return "FS";
        case log_channel_t::GAME:      return "GAME";
        case log_channel_t::TOOLS:     return "TOOLS";
        case log_channel_t::AUDIO:     return "AUDIO";
        case log_channel_t::ASSETS:    return "ASSETS";
        default:                       return "CORE";
    }
}

constexpr inline rcommon::com_u32 Log_ChannelBit( const log_channel_t channel ) {
    return 1u << static_cast<rcommon::com_u32>( channel );
}

};

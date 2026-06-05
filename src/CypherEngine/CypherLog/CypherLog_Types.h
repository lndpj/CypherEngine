#pragma once

#include "CypherEngine/CypherCommon/CypherCommon.h"

#include <ctime>       // std::time_t timestamps.

namespace cypher::engine::log {

constexpr common::usize CYPHER_LOG_MESSAGE_MAX = 1024u;

constexpr common::usize CYPHER_LOG_FILE_PATH_MAX = 512u;

/*
================
Log Types

Severity, output policy, channel and record descriptions for the log system.
================
*/
enum class cypher_log_level_t : common::u8 {
	TRACE,
	DEBUG,
	INFO,
	WARNING,
	ERROR,
	FATAL
};

enum class cypher_log_file_mode_t : common::u8 {
    TRUNCATE,
    APPEND
};

enum class cypher_log_flush_policy_t : common::u8 {
    NEVER,
    ERRORS_AND_ABOVE,
    EVERY_MESSAGE
};

enum class cypher_log_source_path_mode_t : common::u8 {
    BASENAME,
    FULL_PATH
};

enum class cypher_log_channel_t : common::u8 {
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
struct cypher_log_record_t {
	cypher_log_level_t level{ cypher_log_level_t::INFO };
	cypher_log_channel_t channel{ cypher_log_channel_t::CORE };
	const char *file{ "" };
	const char *function{ "" };
	common::com_i32 line{ 0 };
    std::time_t timestamp{};
	char message[CYPHER_LOG_MESSAGE_MAX]{};
};

struct cypher_log_config_t {
	cypher_log_level_t min_level{ cypher_log_level_t::INFO };
	common::com_u32 channel_mask{ 0xFFFFFFFFu };
	bool console_enabled{ true };
    bool file_enabled{ false };
	bool include_timestamps{ true };
    cypher_log_file_mode_t file_mode{ cypher_log_file_mode_t::TRUNCATE };
    cypher_log_flush_policy_t flush_policy{ cypher_log_flush_policy_t::ERRORS_AND_ABOVE };
    cypher_log_source_path_mode_t source_path_mode{ cypher_log_source_path_mode_t::BASENAME };
    char file_path[CYPHER_LOG_FILE_PATH_MAX]{ "reap.log" };
};

/*
================
Log Name Helpers
================
*/
constexpr inline const char *CypherLog_LevelName( const cypher_log_level_t log_level ) {
	switch ( log_level ) {
        case cypher_log_level_t::TRACE:        return "TRACE";
        case cypher_log_level_t::DEBUG:        return "DEBUG";
        case cypher_log_level_t::INFO:         return "INFO";
        case cypher_log_level_t::WARNING:      return "WARNING";
        case cypher_log_level_t::ERROR:        return "ERROR";
        case cypher_log_level_t::FATAL:        return "FATAL";
        default:                        return "UNKNOWN";
	}
}

constexpr inline const char *CypherLog_ChannelName( const cypher_log_channel_t channel ) {
    switch ( channel ) {
        case cypher_log_channel_t::CORE:      return "CORE";
        case cypher_log_channel_t::HOST:      return "HOST";
        case cypher_log_channel_t::PLATFORM:  return "PLATFORM";
        case cypher_log_channel_t::RENDER:    return "RENDER";
        case cypher_log_channel_t::PHYSICS:   return "PHYSICS";
        case cypher_log_channel_t::NET:       return "NET";
        case cypher_log_channel_t::ECS:       return "ECS";
        case cypher_log_channel_t::CFG:       return "CFG";
        case cypher_log_channel_t::CMD:       return "CMD";
        case cypher_log_channel_t::CVAR:      return "CVAR";
        case cypher_log_channel_t::FS:        return "FS";
        case cypher_log_channel_t::GAME:      return "GAME";
        case cypher_log_channel_t::TOOLS:     return "TOOLS";
        case cypher_log_channel_t::AUDIO:     return "AUDIO";
        case cypher_log_channel_t::ASSETS:    return "ASSETS";
        default:                       return "CORE";
    }
}

constexpr inline common::com_u32 CypherLog_ChannelBit( const cypher_log_channel_t channel ) {
    return 1u << static_cast<common::com_u32>( channel );
}

};

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
enum class level_t : common::u8 {
	TRACE,
	DEBUG,
	INFO,
	WARNING,
	ERROR,
	FATAL
};

enum class file_mode_t : common::u8 {
    TRUNCATE,
    APPEND
};

enum class flush_policy_t : common::u8 {
    NEVER,
    ERRORS_AND_ABOVE,
    EVERY_MESSAGE
};

enum class source_path_mode_t : common::u8 {
    BASENAME,
    FULL_PATH
};

enum class channel_t : common::u8 {
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
struct record_t {
	level_t level{ level_t::INFO };
	channel_t channel{ channel_t::CORE };
	const char *file{ "" };
	const char *function{ "" };
	common::com_i32 line{ 0 };
    std::time_t timestamp{};
	char message[CYPHER_LOG_MESSAGE_MAX]{};
};

struct config_t {
	level_t min_level{ level_t::INFO };
	common::com_u32 channel_mask{ 0xFFFFFFFFu };
	bool console_enabled{ true };
    bool file_enabled{ false };
	bool include_timestamps{ true };
    file_mode_t file_mode{ file_mode_t::TRUNCATE };
    flush_policy_t flush_policy{ flush_policy_t::ERRORS_AND_ABOVE };
    source_path_mode_t source_path_mode{ source_path_mode_t::BASENAME };
    char file_path[CYPHER_LOG_FILE_PATH_MAX]{ "reap.log" };
};

/*
================
Log Name Helpers
================
*/
constexpr inline const char *CypherLog_LevelName( const level_t log_level ) {
	switch ( log_level ) {
        case level_t::TRACE:        return "TRACE";
        case level_t::DEBUG:        return "DEBUG";
        case level_t::INFO:         return "INFO";
        case level_t::WARNING:      return "WARNING";
        case level_t::ERROR:        return "ERROR";
        case level_t::FATAL:        return "FATAL";
        default:                        return "UNKNOWN";
	}
}

constexpr inline const char *CypherLog_ChannelName( const channel_t channel ) {
    switch ( channel ) {
        case channel_t::CORE:      return "CORE";
        case channel_t::HOST:      return "HOST";
        case channel_t::PLATFORM:  return "PLATFORM";
        case channel_t::RENDER:    return "RENDER";
        case channel_t::PHYSICS:   return "PHYSICS";
        case channel_t::NET:       return "NET";
        case channel_t::ECS:       return "ECS";
        case channel_t::CFG:       return "CFG";
        case channel_t::CMD:       return "CMD";
        case channel_t::CVAR:      return "CVAR";
        case channel_t::FS:        return "FS";
        case channel_t::GAME:      return "GAME";
        case channel_t::TOOLS:     return "TOOLS";
        case channel_t::AUDIO:     return "AUDIO";
        case channel_t::ASSETS:    return "ASSETS";
        default:                       return "CORE";
    }
}

constexpr inline common::com_u32 CypherLog_ChannelBit( const channel_t channel ) {
    return 1u << static_cast<common::com_u32>( channel );
}

};

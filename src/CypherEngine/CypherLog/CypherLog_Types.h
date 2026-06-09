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
	FATAL,
    COUNT
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

enum class sink_flag_t : common::u32 {
    NONE         = 0u,
    TERMINAL     = 1u << 0u,
    ENGINE_FILE  = 1u << 1u,
    ERROR_FILE   = 1u << 2u,
    CONSOLE_FILE = 1u << 3u,
    EDITOR_FILE  = 1u << 4u,
    GAME_FILE    = 1u << 5u,
    CRASH_BUFFER = 1u << 6u,
    ALL          = 0xFFFFFFFFu
};

enum class format_mode_t : common::u8 {
    COMPACT,
    DETAILED
};

enum class channel_t : common::u8 {
    NONE = 0,
    CORE,
    HOST,
    SYSTEM,
    PLATFORM,
    MEMORY,
    RENDER,
    PHYSICS,
    NET,
    ECS,
    CFG,
    CMD,
    CVAR,
    FS,
    INPUT,
    WORLD,
    RESOURCE,
    SCRIPT,
    ANIMATION,
    AI,
    CONSOLE,
    PROFILE,
    GAME,
    EDITOR,
    TOOLS,
    CLIENT,
    SERVER,
    AUDIO,
    ASSETS,
    MATH,
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
    common::u32 sink_mask{ 0u };

	const char *file{ "" };
	const char *function{ "" };
	common::com_i32 line{ 0 };

    std::time_t timestamp{};
	char message[CYPHER_LOG_MESSAGE_MAX]{};
};

struct sink_config_t {
    bool enabled{ false };
    level_t min_level{ level_t::INFO };
    format_mode_t format{ format_mode_t::COMPACT };
    flush_policy_t flush{ flush_policy_t::ERRORS_AND_ABOVE };
    file_mode_t file{ file_mode_t::TRUNCATE };

    bool include_timestamps{ false };
    bool include_source_location{ false };
    bool include_function_name{ false };
    bool color_enabled{ false };

    char path[CYPHER_LOG_FILE_PATH_MAX]{};
};

struct config_t {
    level_t min_level{ level_t::INFO };
    common::u32 channel_mask{ 0xFFFFFFFFu };
    source_path_mode_t source_path{ source_path_mode_t::BASENAME };

    sink_config_t terminal{
        true,
        level_t::INFO,
        format_mode_t::COMPACT,
        flush_policy_t::ERRORS_AND_ABOVE,
        file_mode_t::TRUNCATE,
        false,
        false,
        false,
        true,
        ""
    };

    sink_config_t engine_file{
        true,
        level_t::TRACE,
        format_mode_t::DETAILED,
        flush_policy_t::ERRORS_AND_ABOVE,
        file_mode_t::TRUNCATE,
        true,
        true,
        true,
        false,
        "CypherEngine.log"
    };

    sink_config_t error_file{
        true,
        level_t::WARNING,
        format_mode_t::DETAILED,
        flush_policy_t::EVERY_MESSAGE,
        file_mode_t::TRUNCATE,
        true,
        true,
        true,
        false,
        "CypherEngine_errors.log"
    };

    sink_config_t console_file{};
    sink_config_t editor_file{};
    sink_config_t game_file{};
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
        default:                    return "UNKNOWN";
	}
}

constexpr inline const char *CypherLog_ChannelName( const channel_t channel ) {
    switch ( channel ) {
        case channel_t::CORE:      return "CORE";
        case channel_t::HOST:      return "HOST";
        case channel_t::SYSTEM:    return "SYSTEM";
        case channel_t::PLATFORM:  return "PLATFORM";
        case channel_t::MEMORY:    return "MEMORY";
        case channel_t::RENDER:    return "RENDER";
        case channel_t::PHYSICS:   return "PHYSICS";
        case channel_t::NET:       return "NET";
        case channel_t::ECS:       return "ECS";
        case channel_t::CFG:       return "CFG";
        case channel_t::CMD:       return "CMD";
        case channel_t::CVAR:      return "CVAR";
        case channel_t::FS:        return "FS";
        case channel_t::INPUT:     return "INPUT";
        case channel_t::WORLD:     return "WORLD";
        case channel_t::RESOURCE:  return "RESOURCE";
        case channel_t::SCRIPT:    return "SCRIPT";
        case channel_t::ANIMATION: return "ANIMATION";
        case channel_t::AI:        return "AI";
        case channel_t::CONSOLE:   return "CONSOLE";
        case channel_t::PROFILE:   return "PROFILE";
        case channel_t::GAME:      return "GAME";
        case channel_t::EDITOR:    return "EDITOR";
        case channel_t::TOOLS:     return "TOOLS";
        case channel_t::CLIENT:    return "CLIENT";
        case channel_t::SERVER:    return "SERVER";
        case channel_t::AUDIO:     return "AUDIO";
        case channel_t::ASSETS:    return "ASSETS";
        case channel_t::MATH:      return "MATH";
        default:                   return "CORE";
    }
}

constexpr inline common::com_u32 CypherLog_ChannelBit( const channel_t channel )
{
    return 1u << static_cast<common::com_u32>( channel );
}

/*
 * Helpers for the sink masking and bit masking.
 */

constexpr inline common::u32 CypherLog_SinkBit( sink_flag_t sink_flag )
{
    return static_cast<common::u32>( sink_flag );
}

constexpr inline bool CypherLog_SinkMaskHas( common::u32 sink_mask, sink_flag_t sink_flag )
{
    return ( ( sink_mask & CypherLog_SinkBit( sink_flag ) ) != 0u );
}

constexpr inline common::u32 CypherLog_SinkMaskAdd( common::u32 sink_mask, sink_flag_t sink_flag )
{
    return sink_mask | CypherLog_SinkBit( sink_flag );
}

constexpr inline common::u32 CypherLog_SinkMaskRemove( common::u32 sink_mask, sink_flag_t sink_flag )
{
    return sink_mask & ~CypherLog_SinkBit( sink_flag );
}

constexpr inline bool CypherLog_LevelPasses( level_t level, level_t min_level )
{
    return static_cast<common::u8>( level ) >= static_cast<common::u8>( min_level );
}

constexpr inline common::com_u32 CypherLog_DefaultSinkMaskForLevel( level_t level )
{
    common::com_u32 mask = 0u;

    mask = CypherLog_SinkMaskAdd( mask, sink_flag_t::TERMINAL );
    mask = CypherLog_SinkMaskAdd( mask, sink_flag_t::ENGINE_FILE );

    if ( CypherLog_LevelPasses( level, level_t::WARNING ) ) {
        mask = CypherLog_SinkMaskAdd( mask, sink_flag_t::ERROR_FILE );
    }

    if ( CypherLog_LevelPasses( level, level_t::ERROR ) ) {
        mask = CypherLog_SinkMaskAdd( mask, sink_flag_t::CRASH_BUFFER );
    }

    return mask;
}

}       // namespace cypher::engine::log

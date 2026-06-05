/*======================================================================
   File: CypherLog.cpp
   Project: CypherEngine
   Author: ksiric <email@example.com>
   Created: 2026-04-19 22:31:16
   Last Modified by: ksiric
   Last Modified: 2026-06-05 12:06:13
   ---------------------------------------------------------------------
   Description:

   ---------------------------------------------------------------------
   License:
   Company:
   Version: 0.1.0
 ======================================================================
                                                                       */
#include "CypherEngine/CypherLog/CypherLog.h"
#include "CypherEngine/CypherSystem/CypherSystem_Platform.h"

#include <cstdio>      // FILE and stdio logging sinks.
#include <cstring>     // strncmp for file path changes.
#include <ctime>       // Timestamps.

namespace cypher::engine::log
{

/*
================
Log Runtime State

Owns the active configuration and optional file sink.
================
*/
struct runtime_state_t {
    config_t config{};
    bool initialized{ false };
    std::FILE *file_handle{ nullptr };
    bool file_error_reported{ false };
};

runtime_state_t g_log_runtime_state_t;

}

namespace cypher::engine::log {

/*
================
CypherLog_Init

Installs the active logging configuration and opens the file sink if requested.
================
*/
error_code_t CypherLog_Init( const config_t &config ){
    if ( g_log_runtime_state_t.file_handle != nullptr ) {
        std::fclose( g_log_runtime_state_t.file_handle );
        g_log_runtime_state_t.file_handle = nullptr;
    }

    g_log_runtime_state_t = {};

    g_log_runtime_state_t.config = config;
    g_log_runtime_state_t.file_error_reported = false;

    if ( g_log_runtime_state_t.config.file_enabled ) {
        const char *open_mode = ( g_log_runtime_state_t.config.file_mode == file_mode_t::APPEND ? "a" : "w" );
        g_log_runtime_state_t.file_handle = std::fopen( g_log_runtime_state_t.config.file_path, open_mode );

        if ( g_log_runtime_state_t.file_handle == nullptr ) {
            g_log_runtime_state_t.initialized = false;
            return error_code_t::ERR_FILE_OPEN_FAILED;
        }
    }

    g_log_runtime_state_t.initialized = true;

    return error_code_t::OK;
}

/*
================
CypherLog_Shutdown
================
*/
void CypherLog_Shutdown( ) {
    if ( g_log_runtime_state_t.file_handle != nullptr ) {
        std::fflush( g_log_runtime_state_t.file_handle );
        std::fclose( g_log_runtime_state_t.file_handle );
    }
    g_log_runtime_state_t.initialized = false;
    g_log_runtime_state_t = {};
}

/*
================
CypherLog_GetConfig
================
*/
const config_t &CypherLog_GetConfig( ) {
    return g_log_runtime_state_t.config;
}

/*
================
CypherLog_SetConfig

Replaces the active logging configuration and rotates file output if needed.
================
*/
error_code_t CypherLog_SetConfig( const config_t &config ) {
    if ( !g_log_runtime_state_t.initialized ) {
        return error_code_t::ERR_NOT_INIT;
    }

    std::FILE *new_file_handle = g_log_runtime_state_t.file_handle;

    const bool file_sink_changed = ( g_log_runtime_state_t.config.file_enabled != config.file_enabled ) || ( g_log_runtime_state_t.config.file_mode != config.file_mode ) || ( std::strncmp( g_log_runtime_state_t.config.file_path, config.file_path, CYPHER_LOG_FILE_PATH_MAX ) != 0 );

    if ( file_sink_changed ) {

        new_file_handle = nullptr;

        if ( config.file_enabled ) {
            const char *open_mode = ( config.file_mode == file_mode_t::APPEND ) ? "a" : "w";

            new_file_handle = std::fopen( config.file_path, open_mode );

            if ( new_file_handle == nullptr ) {
                return error_code_t::ERR_FILE_OPEN_FAILED;
            }
        }
    }

    if ( file_sink_changed && g_log_runtime_state_t.file_handle != nullptr ) {
        std::fflush( g_log_runtime_state_t.file_handle );
        std::fclose( g_log_runtime_state_t.file_handle );
    }

    g_log_runtime_state_t.config = config;
    g_log_runtime_state_t.file_handle = new_file_handle;
    g_log_runtime_state_t.file_error_reported = false;

    return error_code_t::OK;
}

/*
================
CypherLog_LevelEnabled

Checks severity and channel filters before building a log record.
================
*/
bool CypherLog_LevelEnabled( const level_t level, const channel_t channel ) {
    if ( !g_log_runtime_state_t.initialized ) {
        return false;
    }

    if ( channel == channel_t::NONE || channel == channel_t::COUNT ) {
        return false;
    }

    const auto level_as_int = static_cast<common::com_u32>( level );
    const auto min_level_as_int = static_cast<common::com_u32>( g_log_runtime_state_t.config.min_level );

    if ( level_as_int < min_level_as_int ) {
        return false;
    }

    return CypherLog_ChannelEnabled( g_log_runtime_state_t.config.channel_mask, channel );
}

/*
================
CypherLog_ChannelEnabled

Checks a channel bit against the active channel mask.
================
*/
bool CypherLog_ChannelEnabled( const common::com_u32 channel_mask, const channel_t channel ) {
    if ( channel == channel_t::NONE || channel == channel_t::COUNT ) {
        return false;
    }
    
    const auto channel_as_int = static_cast<common::com_u32>( channel );

    if ( channel_as_int >= 32 ) {
        return false;
    }

    return ( channel_mask & ( 1u << channel_as_int ) ) != 0u;
}

/*
================
CypherLog_ShouldFlush
================
*/
bool CypherLog_ShouldFlush( const flush_policy_t flush_policy, const level_t level ) {

    switch ( flush_policy ) {
        case flush_policy_t::NEVER:
            return false;
        case flush_policy_t::ERRORS_AND_ABOVE:
            return static_cast<common::u32>( level ) >= static_cast<common::u32>( level_t::ERROR );
        case flush_policy_t::EVERY_MESSAGE:
            return true;
        default:
            return false;
    }
}

/*
================
CypherLog_Emit

Writes a fully built log record to console and/or file sinks.
================
*/
void CypherLog_Emit( const record_t &record ) {
    if ( record.message[0] == '\0' ) {
        return;
    }

    if ( !CypherLog_LevelEnabled( record.level, record.channel ) ) {
        return ;
    }

    const auto &cfg = CypherLog_GetConfig();

    // If no sink is enabled, the record is intentionally dropped.
    if ( !cfg.console_enabled && !( cfg.file_enabled && g_log_runtime_state_t.file_handle != nullptr ) ) {
        return ;
    }

    const char *source_file = record.file ? record.file : "<unknown_file>";

    if ( cfg.source_path_mode == source_path_mode_t::BASENAME ) {
        source_file = sys::CypherSystem_PathBasename( source_file );
    }

    char timestamp_buffer[32]{};

    if ( cfg.include_timestamps && record.timestamp != std::time_t{} ) {
        std::tm tm_value{};

        if ( sys::CypherSystem_LocalTime( record.timestamp, tm_value ) ) {
            std::strftime( timestamp_buffer, sizeof( timestamp_buffer ), "%H:%M:%S", &tm_value );
        }
    }

    char line_buffer[CYPHER_LOG_MESSAGE_MAX + 256]{};
    
    if ( timestamp_buffer[0] != '\0' ) {
        std::snprintf(
                      line_buffer,
                      sizeof( line_buffer ),
                      "[%s][%s][%s] %s:%d (%s) %s\n",
                      timestamp_buffer,
                      CypherLog_LevelName( record.level ),
                      CypherLog_ChannelName( record.channel ),
                      source_file,
                      record.line,
                      record.function ? record.function : "<unknown_function>",
                      record.message
                      );
    } else {
        std::snprintf(
                    line_buffer,
                    sizeof( line_buffer ),
                    "[%s][%s] %s:%d (%s) %s\n",
                    CypherLog_LevelName( record.level ),
                    CypherLog_ChannelName( record.channel ),
                    source_file,
                    record.line,
                    record.function ? record.function : "<unknown_function>",
                    record.message
                    );
    }

    if ( cfg.console_enabled ) {
        std::fputs( line_buffer, stdout );
    }

    if ( cfg.file_enabled && g_log_runtime_state_t.file_handle != nullptr ) {
        std::fputs( line_buffer, g_log_runtime_state_t.file_handle );
    }

    if ( CypherLog_ShouldFlush( cfg.flush_policy, record.level ) ) {

        if ( cfg.console_enabled ) {
            std::fflush( stdout );
        }

        if ( cfg.file_enabled && g_log_runtime_state_t.file_handle != nullptr ) {
            std::fflush( g_log_runtime_state_t.file_handle );
        }
    }
}

/*
================
CypherLog_Emitf

Formats and emits a log event from variadic arguments.
================
*/
void CypherLog_Emitf( const level_t level, const channel_t channel,
                const char *file, const char *function, const common::com_i32 line,
                const char *format, ... ) {
    if ( !CypherLog_LevelEnabled( level, channel ) ) {
        return ;
    }
    va_list args;
    va_start( args, format );
    CypherLog_Emitfv( level, channel, file, function, line, format, args );
    va_end( args );
}

/*
================
CypherLog_Emitfv

Builds a log record from an existing va_list.
================
*/
void CypherLog_Emitfv( const level_t level, const channel_t channel,
                 const char *file, const char *function, const common::com_i32 line,
                 const char *format, va_list args ) {

    if ( !CypherLog_LevelEnabled( level, channel ) ) {
        return ;
    }

    record_t record{};
    record.level = level;
    record.channel = channel;
    record.file = file ? file : "<unknown_file>";
    record.function = function ? function : "<unknown_function>";
    record.line = line;
    record.timestamp = std::time( nullptr );

    const char *safe_format = format ? format : "<null format>";
    std::vsnprintf( record.message, sizeof ( record.message ), safe_format, args );

    CypherLog_Emit( record );
}

}       // namespace cypher::engine::log

/*======================================================================
   File: CypherLog.cpp
   Project: CypherEngine
   Author: ksiric <email@example.com>
   Created: 2026-04-19 22:31:16
   Last Modified by: ksiric
   Last Modified: 2026-06-09 20:31:37
   ---------------------------------------------------------------------
   Description:

   ---------------------------------------------------------------------
   License:
   Company:
   Version: 0.1.0
 ======================================================================
                                                                       */
#include "CypherEngine/CypherLog/CypherLog.h"
#include "CypherEngine/CypherLog/CypherLog_Format.h"

#include <cctype>      // std::tolower for level parsing.
#include <cstdio>      // FILE and stdio logging sinks.
#include <ctime>       // std::time.
#include <cstring>     // std::strcmp / std::strncmp for level and sink config checks.

namespace cypher::engine::log
{

/*
================
Log Runtime State

Owns active configuration and concrete file handles for enabled file sinks.
================
*/
struct runtime_state_t {
    config_t config{};
    bool initialized{ false };

    std::FILE *engine_file_handle{ nullptr };
    std::FILE *error_file_handle{ nullptr };
    std::FILE *console_file_handle{ nullptr };
    std::FILE *editor_file_handle{ nullptr };
    std::FILE *game_file_handle{ nullptr };

    bool file_error_reported{ false };
};

runtime_state_t g_log_runtime_state_t;

}

namespace cypher::engine::log
{

/*
================
CypherLog_FileOpenMode
================
*/
const char *CypherLog_FileOpenMode( const file_mode_t file_mode )
{
    return ( file_mode == file_mode_t::APPEND ) ? "a" : "w";
}

/*
================
CypherLog_CloseFile
================
*/
void CypherLog_CloseFile( std::FILE *&file_handle )
{
    if ( file_handle == nullptr ) {
        return;
    }

    std::fflush( file_handle );
    std::fclose( file_handle );
    file_handle = nullptr;
}

/*
================
CypherLog_CloseFileSinks
================
*/
void CypherLog_CloseFileSinks( runtime_state_t &runtime_state )
{
    CypherLog_CloseFile( runtime_state.engine_file_handle );
    CypherLog_CloseFile( runtime_state.error_file_handle );
    CypherLog_CloseFile( runtime_state.console_file_handle );
    CypherLog_CloseFile( runtime_state.editor_file_handle );
    CypherLog_CloseFile( runtime_state.game_file_handle );
}

/*
================
CypherLog_OpenFileSink
================
*/
log_error_t CypherLog_OpenFileSink( const sink_config_t &sink_config, std::FILE *&file_handle )
{
    file_handle = nullptr;

    if ( !sink_config.enabled ) {
        return log_error_t::OK;
    }

    if ( sink_config.path[0] == '\0' ) {
        return log_error_t::ERR_INVALID_CONFIG;
    }

    file_handle = std::fopen( sink_config.path, CypherLog_FileOpenMode( sink_config.file ) );

    if ( file_handle == nullptr ) {
        return log_error_t::ERR_FILE_OPEN_FAILED;
    }

    return log_error_t::OK;
}

/*
================
CypherLog_FileSinkSameTarget
================
*/
bool CypherLog_FileSinkSameTarget( const sink_config_t &old_config, const sink_config_t &new_config )
{
    if ( old_config.enabled != new_config.enabled ) {
        return false;
    }

    if ( old_config.file != new_config.file ) {
        return false;
    }

    return std::strncmp( old_config.path, new_config.path, CYPHER_LOG_FILE_PATH_MAX ) == 0;
}

/*
================
CypherLog_UpdateFileSink

Keeps unchanged file handles alive. This avoids truncating the active log file
when runtime cvars are applied without changing the sink target.
================
*/
log_error_t CypherLog_UpdateFileSink( const sink_config_t &old_config, const sink_config_t &new_config, std::FILE *&file_handle )
{
    if ( !new_config.enabled ) {
        CypherLog_CloseFile( file_handle );
        return log_error_t::OK;
    }

    if ( file_handle != nullptr && CypherLog_FileSinkSameTarget( old_config, new_config ) ) {
        return log_error_t::OK;
    }

    CypherLog_CloseFile( file_handle );
    return CypherLog_OpenFileSink( new_config, file_handle );
}

/*
================
CypherLog_OpenFileSinks
================
*/
log_error_t CypherLog_OpenFileSinks(
    const config_t &config,
    std::FILE *&engine_file_handle,
    std::FILE *&error_file_handle,
    std::FILE *&console_file_handle,
    std::FILE *&editor_file_handle,
    std::FILE *&game_file_handle )
{
    log_error_t result = log_error_t::OK;

    result = CypherLog_OpenFileSink( config.engine_file, engine_file_handle );
    if ( result != log_error_t::OK ) {
        return result;
    }

    result = CypherLog_OpenFileSink( config.error_file, error_file_handle );
    if ( result != log_error_t::OK ) {
        CypherLog_CloseFile( engine_file_handle );
        return result;
    }

    result = CypherLog_OpenFileSink( config.console_file, console_file_handle );
    if ( result != log_error_t::OK ) {
        CypherLog_CloseFile( engine_file_handle );
        CypherLog_CloseFile( error_file_handle );
        return result;
    }

    result = CypherLog_OpenFileSink( config.editor_file, editor_file_handle );
    if ( result != log_error_t::OK ) {
        CypherLog_CloseFile( engine_file_handle );
        CypherLog_CloseFile( error_file_handle );
        CypherLog_CloseFile( console_file_handle );
        return result;
    }

    result = CypherLog_OpenFileSink( config.game_file, game_file_handle );
    if ( result != log_error_t::OK ) {
        CypherLog_CloseFile( engine_file_handle );
        CypherLog_CloseFile( error_file_handle );
        CypherLog_CloseFile( console_file_handle );
        CypherLog_CloseFile( editor_file_handle );
        return result;
    }

    return log_error_t::OK;
}

/*
================
CypherLog_ShouldFlush
================
*/
bool CypherLog_ShouldFlush( const flush_policy_t flush_policy, const level_t level )
{
    switch ( flush_policy ) {
        case flush_policy_t::NEVER:
            return false;
        case flush_policy_t::ERRORS_AND_ABOVE:
            return CypherLog_LevelPasses( level, level_t::ERROR );
        case flush_policy_t::EVERY_MESSAGE:
            return true;
        default:
            return false;
    }
}

/*
================
CypherLog_SinkAcceptsRecord
================
*/
bool CypherLog_SinkAcceptsRecord( const record_t &record, const sink_config_t &sink_config )
{
    if ( !sink_config.enabled ) {
        return false;
    }

    return CypherLog_LevelPasses( record.level, sink_config.min_level );
}

/*
================
CypherLog_TerminalStreamForLevel
================
*/
std::FILE *CypherLog_TerminalStreamForLevel( const level_t level )
{
    return CypherLog_LevelPasses( level, level_t::WARNING ) ? stderr : stdout;
}

/*
================
CypherLog_WriteFormattedRecord
================
*/
void CypherLog_WriteFormattedRecord(
    const record_t &record,
    const sink_config_t &sink_config,
    const config_t &config,
    std::FILE *file_handle )
{
    if ( file_handle == nullptr ) {
        return;
    }

    char line_buffer[CYPHER_LOG_MESSAGE_MAX + 512u]{};
    const log_error_t format_result = CypherLog_FormatRecord(
        record,
        sink_config,
        config,
        line_buffer,
        sizeof( line_buffer )
    );

    if ( format_result != log_error_t::OK ) {
        if ( !g_log_runtime_state_t.file_error_reported ) {
            std::fputs( "[ERROR][LOG] failed formatting log record.\n", stderr );
            g_log_runtime_state_t.file_error_reported = true;
        }

        return;
    }

    if ( std::fputs( line_buffer, file_handle ) < 0 ) {
        if ( !g_log_runtime_state_t.file_error_reported ) {
            std::fputs( "[ERROR][LOG] failed writing log record.\n", stderr );
            g_log_runtime_state_t.file_error_reported = true;
        }

        return;
    }

    if ( CypherLog_ShouldFlush( sink_config.flush, record.level ) ) {
        std::fflush( file_handle );
    }
}

/*
================
CypherLog_Init

Installs active logging configuration and opens enabled file sinks.
================
*/
log_error_t CypherLog_Init( const config_t &config )
{
    CypherLog_CloseFileSinks( g_log_runtime_state_t );
    g_log_runtime_state_t = {};

    std::FILE *engine_file_handle = nullptr;
    std::FILE *error_file_handle = nullptr;
    std::FILE *console_file_handle = nullptr;
    std::FILE *editor_file_handle = nullptr;
    std::FILE *game_file_handle = nullptr;

    const log_error_t open_result = CypherLog_OpenFileSinks(
        config,
        engine_file_handle,
        error_file_handle,
        console_file_handle,
        editor_file_handle,
        game_file_handle
    );

    if ( open_result != log_error_t::OK ) {
        return open_result;
    }

    g_log_runtime_state_t.config = config;
    g_log_runtime_state_t.engine_file_handle = engine_file_handle;
    g_log_runtime_state_t.error_file_handle = error_file_handle;
    g_log_runtime_state_t.console_file_handle = console_file_handle;
    g_log_runtime_state_t.editor_file_handle = editor_file_handle;
    g_log_runtime_state_t.game_file_handle = game_file_handle;
    g_log_runtime_state_t.initialized = true;
    g_log_runtime_state_t.file_error_reported = false;

    return log_error_t::OK;
}

/*
================
CypherLog_Shutdown
================
*/
void CypherLog_Shutdown()
{
    CypherLog_CloseFileSinks( g_log_runtime_state_t );
    g_log_runtime_state_t.initialized = false;
    g_log_runtime_state_t = {};
}

/*
================
CypherLog_GetConfig
================
*/
const config_t &CypherLog_GetConfig()
{
    return g_log_runtime_state_t.config;
}

/*
================
CypherLog_IsInitialized
================
*/
bool CypherLog_IsInitialized()
{
    return g_log_runtime_state_t.initialized;
}

/*
================
CypherLog_LevelFromString

Parses config/console level strings into logger severity values.
================
*/
log_error_t CypherLog_LevelFromString( const char *level_name, level_t &out_level )
{
    if ( level_name == nullptr || level_name[0] == '\0' ) {
        return log_error_t::ERR_INVALID_LEVEL;
    }

    char lower[32]{};
    common::u32 i = 0u;

    for ( ; i + 1u < sizeof( lower ) && level_name[i] != '\0'; ++i ) {
        lower[i] = static_cast<char>( std::tolower( static_cast<unsigned char>( level_name[i] ) ) );
    }

    lower[i] = '\0';

    if ( std::strcmp( lower, "trace" ) == 0 || std::strcmp( lower, "0" ) == 0 ) {
        out_level = level_t::TRACE;
        return log_error_t::OK;
    }

    if ( std::strcmp( lower, "debug" ) == 0 || std::strcmp( lower, "1" ) == 0 ) {
        out_level = level_t::DEBUG;
        return log_error_t::OK;
    }

    if ( std::strcmp( lower, "info" ) == 0 || std::strcmp( lower, "2" ) == 0 ) {
        out_level = level_t::INFO;
        return log_error_t::OK;
    }

    if ( std::strcmp( lower, "warning" ) == 0 || std::strcmp( lower, "warn" ) == 0 || std::strcmp( lower, "3" ) == 0 ) {
        out_level = level_t::WARNING;
        return log_error_t::OK;
    }

    if ( std::strcmp( lower, "error" ) == 0 || std::strcmp( lower, "4" ) == 0 ) {
        out_level = level_t::ERROR;
        return log_error_t::OK;
    }

    if ( std::strcmp( lower, "fatal" ) == 0 || std::strcmp( lower, "5" ) == 0 ) {
        out_level = level_t::FATAL;
        return log_error_t::OK;
    }

    return log_error_t::ERR_INVALID_LEVEL;
}

/*
================
CypherLog_SetConfig

Replaces active configuration and rotates file sinks if requested.
================
*/
log_error_t CypherLog_SetConfig( const config_t &config )
{
    if ( !g_log_runtime_state_t.initialized ) {
        return log_error_t::ERR_NOT_INIT;
    }

    log_error_t update_result = log_error_t::OK;

    update_result = CypherLog_UpdateFileSink( g_log_runtime_state_t.config.engine_file, config.engine_file, g_log_runtime_state_t.engine_file_handle );
    if ( update_result != log_error_t::OK ) {
        return update_result;
    }

    update_result = CypherLog_UpdateFileSink( g_log_runtime_state_t.config.error_file, config.error_file, g_log_runtime_state_t.error_file_handle );
    if ( update_result != log_error_t::OK ) {
        return update_result;
    }

    update_result = CypherLog_UpdateFileSink( g_log_runtime_state_t.config.console_file, config.console_file, g_log_runtime_state_t.console_file_handle );
    if ( update_result != log_error_t::OK ) {
        return update_result;
    }

    update_result = CypherLog_UpdateFileSink( g_log_runtime_state_t.config.editor_file, config.editor_file, g_log_runtime_state_t.editor_file_handle );
    if ( update_result != log_error_t::OK ) {
        return update_result;
    }

    update_result = CypherLog_UpdateFileSink( g_log_runtime_state_t.config.game_file, config.game_file, g_log_runtime_state_t.game_file_handle );
    if ( update_result != log_error_t::OK ) {
        return update_result;
    }

    g_log_runtime_state_t.config = config;
    g_log_runtime_state_t.file_error_reported = false;

    return log_error_t::OK;
}

/*
================
CypherLog_LevelEnabled

Checks global severity and channel filters before building a log record.
================
*/
bool CypherLog_LevelEnabled( const level_t level, const channel_t channel )
{
    if ( !g_log_runtime_state_t.initialized ) {
        return false;
    }

    if ( channel == channel_t::NONE || channel == channel_t::COUNT ) {
        return false;
    }

    if ( level == level_t::COUNT ) {
        return false;
    }

    if ( !CypherLog_LevelPasses( level, g_log_runtime_state_t.config.min_level ) ) {
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
bool CypherLog_ChannelEnabled( const common::com_u32 channel_mask, const channel_t channel )
{
    if ( channel == channel_t::NONE || channel == channel_t::COUNT ) {
        return false;
    }

    const auto channel_as_int = static_cast<common::com_u32>( channel );

    if ( channel_as_int >= 32u ) {
        return false;
    }

    return ( channel_mask & CypherLog_ChannelBit( channel ) ) != 0u;
}

/*
================
CypherLog_Emit

Routes a fully built log record to all enabled sinks requested by its sink mask.
================
*/
void CypherLog_Emit( const record_t &record )
{
    if ( record.message[0] == '\0' ) {
        return;
    }

    if ( !CypherLog_LevelEnabled( record.level, record.channel ) ) {
        return;
    }

    const config_t &config = CypherLog_GetConfig();
    const common::u32 sink_mask = ( record.sink_mask != 0u ) ? record.sink_mask : CypherLog_DefaultSinkMaskForLevel( record.level );

    if ( CypherLog_SinkMaskHas( sink_mask, sink_flag_t::TERMINAL ) && CypherLog_SinkAcceptsRecord( record, config.terminal ) ) {
        CypherLog_WriteFormattedRecord( record, config.terminal, config, CypherLog_TerminalStreamForLevel( record.level ) );
    }

    if ( CypherLog_SinkMaskHas( sink_mask, sink_flag_t::ENGINE_FILE ) && CypherLog_SinkAcceptsRecord( record, config.engine_file ) ) {
        CypherLog_WriteFormattedRecord( record, config.engine_file, config, g_log_runtime_state_t.engine_file_handle );
    }

    if ( CypherLog_SinkMaskHas( sink_mask, sink_flag_t::ERROR_FILE ) && CypherLog_SinkAcceptsRecord( record, config.error_file ) ) {
        CypherLog_WriteFormattedRecord( record, config.error_file, config, g_log_runtime_state_t.error_file_handle );
    }

    if ( CypherLog_SinkMaskHas( sink_mask, sink_flag_t::CONSOLE_FILE ) && CypherLog_SinkAcceptsRecord( record, config.console_file ) ) {
        CypherLog_WriteFormattedRecord( record, config.console_file, config, g_log_runtime_state_t.console_file_handle );
    }

    if ( CypherLog_SinkMaskHas( sink_mask, sink_flag_t::EDITOR_FILE ) && CypherLog_SinkAcceptsRecord( record, config.editor_file ) ) {
        CypherLog_WriteFormattedRecord( record, config.editor_file, config, g_log_runtime_state_t.editor_file_handle );
    }

    if ( CypherLog_SinkMaskHas( sink_mask, sink_flag_t::GAME_FILE ) && CypherLog_SinkAcceptsRecord( record, config.game_file ) ) {
        CypherLog_WriteFormattedRecord( record, config.game_file, config, g_log_runtime_state_t.game_file_handle );
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
                const char *format, ... )
{
    if ( !CypherLog_LevelEnabled( level, channel ) ) {
        return;
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
                 const char *format, va_list args )
{
    if ( !CypherLog_LevelEnabled( level, channel ) ) {
        return;
    }

    record_t record{};
    record.level = level;
    record.channel = channel;
    record.sink_mask = CypherLog_DefaultSinkMaskForLevel( level );
    record.file = file ? file : "<unknown_file>";
    record.function = function ? function : "<unknown_function>";
    record.line = line;
    record.timestamp = std::time( nullptr );

    const char *safe_format = format ? format : "<null format>";
    std::vsnprintf( record.message, sizeof( record.message ), safe_format, args );

    CypherLog_Emit( record );
}

}       // namespace cypher::engine::log

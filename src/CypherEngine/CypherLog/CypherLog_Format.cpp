#include "CypherEngine/CypherLog/CypherLog_Format.h"
#include "CypherEngine/CypherSystem/CypherSystem_Platform.h"

#include <cstdio>      // std::snprintf.
#include <ctime>       // std::strftime.

namespace cypher::engine::log
{

/*
================
CypherLog_LevelColor

Returns ANSI terminal color for compact terminal output. File sinks keep colors
disabled so escape sequences do not pollute log files.
================
*/
const char *CypherLog_LevelColor( const level_t level )
{
    switch ( level ) {
        case level_t::TRACE:   return "\033[90m";
        case level_t::DEBUG:   return "\033[36m";
        case level_t::INFO:    return "\033[37m";
        case level_t::WARNING: return "\033[33m";
        case level_t::ERR:     return "\033[31m";
        case level_t::FATAL:   return "\033[35m";
        default:               return "";
    }
}

/*
================
CypherLog_FormatTimestamp
================
*/
bool CypherLog_FormatTimestamp( const record_t &record, char *out_buffer, const common::usize out_buffer_size )
{
    if ( out_buffer == nullptr || out_buffer_size == 0u || record.timestamp == std::time_t{} ) {
        return false;
    }

    std::tm tm_value{};

    if ( !sys::CypherSystem_LocalTime( record.timestamp, tm_value ) ) {
        return false;
    }

    return std::strftime( out_buffer, out_buffer_size, "%H:%M:%S", &tm_value ) > 0u;
}

/*
================
CypherLog_FormatCompact
================
*/
log_error_t CypherLog_FormatCompact(
    const record_t &record,
    const sink_config_t &sink_config,
    char *out_buffer,
    const common::usize out_buffer_size )
{
    const char *color_begin = sink_config.color_enabled ? CypherLog_LevelColor( record.level ) : "";
    const char *color_end = sink_config.color_enabled ? "\033[0m" : "";

    const int written = std::snprintf(
        out_buffer,
        out_buffer_size,
        "%s[%s][%s] %s%s\n",
        color_begin,
        CypherLog_LevelName( record.level ),
        CypherLog_ChannelName( record.channel ),
        record.message,
        color_end
    );

    return ( written < 0 ) ? log_error_t::ERR_FORMAT_FAILED : log_error_t::OK;
}

/*
================
CypherLog_FormatDetailed
================
*/
log_error_t CypherLog_FormatDetailed(
    const record_t &record,
    const sink_config_t &sink_config,
    const config_t &config,
    char *out_buffer,
    const common::usize out_buffer_size )
{
    char timestamp_buffer[32]{};

    if ( sink_config.include_timestamps ) {
        CypherLog_FormatTimestamp( record, timestamp_buffer, sizeof( timestamp_buffer ) );
    }

    const bool has_source_location = record.file != nullptr && record.file[0] != '\0';
    const bool has_function_name = record.function != nullptr && record.function[0] != '\0';
    const bool include_source_location = sink_config.include_source_location && has_source_location;
    const bool include_function_name = sink_config.include_function_name && has_function_name;

    const char *source_file = has_source_location ? record.file : "";

    if ( include_source_location && config.source_path == source_path_mode_t::BASENAME ) {
        source_file = sys::CypherSystem_PathBasename( source_file );
    }

    const char *function_name = has_function_name ? record.function : "";

    int written = 0;

    if ( timestamp_buffer[0] != '\0' && include_source_location && include_function_name ) {
        written = std::snprintf(
            out_buffer,
            out_buffer_size,
            "[%s][%s][%s] %s:%d (%s) %s\n",
            timestamp_buffer,
            CypherLog_LevelName( record.level ),
            CypherLog_ChannelName( record.channel ),
            source_file,
            record.line,
            function_name,
            record.message
        );
    } else if ( timestamp_buffer[0] != '\0' && include_source_location ) {
        written = std::snprintf(
            out_buffer,
            out_buffer_size,
            "[%s][%s][%s] %s:%d %s\n",
            timestamp_buffer,
            CypherLog_LevelName( record.level ),
            CypherLog_ChannelName( record.channel ),
            source_file,
            record.line,
            record.message
        );
    } else if ( timestamp_buffer[0] != '\0' ) {
        written = std::snprintf(
            out_buffer,
            out_buffer_size,
            "[%s][%s][%s] %s\n",
            timestamp_buffer,
            CypherLog_LevelName( record.level ),
            CypherLog_ChannelName( record.channel ),
            record.message
        );
    } else if ( include_source_location && include_function_name ) {
        written = std::snprintf(
            out_buffer,
            out_buffer_size,
            "[%s][%s] %s:%d (%s) %s\n",
            CypherLog_LevelName( record.level ),
            CypherLog_ChannelName( record.channel ),
            source_file,
            record.line,
            function_name,
            record.message
        );
    } else if ( include_source_location ) {
        written = std::snprintf(
            out_buffer,
            out_buffer_size,
            "[%s][%s] %s:%d %s\n",
            CypherLog_LevelName( record.level ),
            CypherLog_ChannelName( record.channel ),
            source_file,
            record.line,
            record.message
        );
    } else {
        written = std::snprintf(
            out_buffer,
            out_buffer_size,
            "[%s][%s] %s\n",
            CypherLog_LevelName( record.level ),
            CypherLog_ChannelName( record.channel ),
            record.message
        );
    }

    return ( written < 0 ) ? log_error_t::ERR_FORMAT_FAILED : log_error_t::OK;
}

/*
================
CypherLog_FormatRecord
================
*/
log_error_t CypherLog_FormatRecord(
    const record_t &record,
    const sink_config_t &sink_config,
    const config_t &config,
    char *out_buffer,
    const common::usize out_buffer_size )
{
    if ( out_buffer == nullptr || out_buffer_size == 0u ) {
        return log_error_t::ERR_FORMAT_FAILED;
    }

    out_buffer[0] = '\0';

    if ( record.message[0] == '\0' ) {
        return log_error_t::ERR_FORMAT_FAILED;
    }

    switch ( sink_config.format ) {
        case format_mode_t::COMPACT:
            return CypherLog_FormatCompact( record, sink_config, out_buffer, out_buffer_size );
        case format_mode_t::DETAILED:
            return CypherLog_FormatDetailed( record, sink_config, config, out_buffer, out_buffer_size );
        default:
            return log_error_t::ERR_FORMAT_FAILED;
    }
}

}       // namespace cypher::engine::log

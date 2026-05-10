#pragma once

#include "rengine/log/log_error.h"
#include "rengine/log/log_types.h"

#include <cstdarg>     // va_list for log formatting.

namespace reap::rengine::log
{

/*
================
Log API

Structured logging with severity levels, channels and optional file output.
================
*/
log_error_code_t Log_Init( const log_config_t &config = {} );

void Log_Shutdown();

const log_config_t &Log_GetConfig();

log_error_code_t Log_SetConfig( const log_config_t &config );

bool Log_LevelEnabled( const log_level_t log_level, const log_channel_t channel );

bool Log_ChannelEnabled( const rcommon::com_u32 channel_mask, const log_channel_t channel );

void Log_Emit( const log_record_t &record );

void Log_Emitf( const log_level_t log_level, const log_channel_t channel,
                const char *file, const char *function, const rcommon::com_i32 line,
                const char *format, ... );

void Log_Emitfv( const log_level_t log_level, const log_channel_t channel,
                 const char *file, const char *function, const rcommon::com_i32 line,
                 const char *format, va_list args );

}

/*
================
Log Macros

Capture source file, function and line while keeping call sites compact.
================
*/
#define REAP_LOG_IF_ENABLED( LOG_LEVEL, LOG_CHANNEL )                                                           \
	if ( reap::rengine::log::Log_LevelEnabled( ( LOG_LEVEL ), ( LOG_CHANNEL ) ) )

#define REAP_LOG( LOG_LEVEL, LOG_CHANNEL, LOG_MESSAGE )                                                         \
    REAP_LOGF( ( LOG_LEVEL ), ( LOG_CHANNEL ), "%s", ( LOG_MESSAGE ) )

#define REAP_LOGF( LOG_LEVEL, LOG_CHANNEL, LOG_FORMAT, ... )                                                    \
    do {                                                                                                        \
        if ( reap::rengine::log::Log_LevelEnabled( ( LOG_LEVEL), ( LOG_CHANNEL ) ) ) {                          \
            reap::rengine::log::Log_Emitf(                                                                      \
                ( LOG_LEVEL ), ( LOG_CHANNEL ), __FILE__, __func__, __LINE__, ( LOG_FORMAT )                     \
				__VA_OPT__( , ) __VA_ARGS__ );                                                             \
        }                                                                                                       \
    } while ( false );

#define REAP_LOGF_IF( LOG_CONDITION, LOG_LEVEL, LOG_CHANNEL, LOG_FORMAT, ... )                                  \
    do {                                                                                                        \
        if ( ( LOG_CONDITION ) && reap::rengine::log::Log_LevelEnabled( ( LOG_LEVEL ), (LOG_CHANNEL ) ) ) {    \
            reap::rengine::log::Log_Emitf(                                                                      \
                ( LOG_LEVEL ), ( LOG_CHANNEL ), __FILE__, __func__, __LINE__, ( LOG_FORMAT )                    \
				__VA_OPT__( , ) __VA_ARGS__ );                                                             \
        }                                                                                                       \
    } while ( false );

#define REAP_LOG_TRACE( LOG_CHANNEL, LOG_FORMAT, ... )          REAP_LOGF( reap::rengine::log::log_level_t::TRACE, ( LOG_CHANNEL ), ( LOG_FORMAT ) __VA_OPT__( , ) __VA_ARGS__ )

#define REAP_LOG_DEBUG( LOG_CHANNEL, LOG_FORMAT, ... )          REAP_LOGF( reap::rengine::log::log_level_t::DEBUG, ( LOG_CHANNEL ), ( LOG_FORMAT ) __VA_OPT__( , ) __VA_ARGS__ )

#define REAP_LOG_INFO( LOG_CHANNEL, LOG_FORMAT, ... )           REAP_LOGF( reap::rengine::log::log_level_t::INFO, ( LOG_CHANNEL ), ( LOG_FORMAT ) __VA_OPT__( , ) __VA_ARGS__ )

#define REAP_LOG_WARNING( LOG_CHANNEL, LOG_FORMAT, ... )        REAP_LOGF( reap::rengine::log::log_level_t::WARNING, ( LOG_CHANNEL ), ( LOG_FORMAT ) __VA_OPT__( , ) __VA_ARGS__ )

#define REAP_LOG_ERROR( LOG_CHANNEL, LOG_FORMAT, ... )          REAP_LOGF( reap::rengine::log::log_level_t::ERROR, ( LOG_CHANNEL ), ( LOG_FORMAT ) __VA_OPT__( , ) __VA_ARGS__ )

#define REAP_LOG_FATAL( LOG_CHANNEL, LOG_FORMAT, ... )          REAP_LOGF( reap::rengine::log::log_level_t::FATAL, ( LOG_CHANNEL ), ( LOG_FORMAT ) __VA_OPT__( , ) __VA_ARGS__ )

#define REAP_LOG_INFO_MESSAGE( LOG_CHANNEL, LOG_MESSAGE )       REAP_LOG( reap::rengine::log::log_level_t::INFO, ( LOG_CHANNEL ), ( LOG_MESSAGE ) )

#define REAP_LOG_WARNING_MESSAGE( LOG_CHANNEL, LOG_MESSAGE )    REAP_LOG( reap::rengine::log::log_level_t::WARNING, ( LOG_CHANNEL ), ( LOG_MESSAGE ) )

#define REAP_LOG_ERROR_MESSAGE( LOG_CHANNEL, LOG_MESSAGE )      REAP_LOG( reap::rengine::log::log_level_t::ERROR, ( LOG_CHANNEL ), ( LOG_MESSAGE ) )

#define REAP_LOG_CHECK( CONDITION, LOG_CHANNEL, LOG_FORMAT, ... )                                                                   \
    do {                                                                                                                            \
        if ( !CONDITION ) {                                                                                                         \
            REAP_LOG_ERROR( ( LOG_CHANNEL ), ( LOG_FORMAT ) __VA_OPT__( , ) __VA_ARGS__ );                                         \
        }                                                                                                                           \
    } while ( false );

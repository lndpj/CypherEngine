#pragma once

#include "CypherEngine/CypherLog/CypherLog_Error.h"
#include "CypherEngine/CypherLog/CypherLog_Types.h"

#include <cstdarg>     // va_list for log formatting.

namespace cypher::engine::log
{

/*
================
Log API

Structured logging with severity levels, channels and optional file output.
================
*/
error_code_t CypherLog_Init( const config_t &config = {} );

void CypherLog_Shutdown();

const config_t &CypherLog_GetConfig();

error_code_t CypherLog_SetConfig( const config_t &config );

bool CypherLog_LevelEnabled( const level_t log_level, const channel_t channel );

bool CypherLog_ChannelEnabled( const common::com_u32 channel_mask, const channel_t channel );

void CypherLog_Emit( const record_t &record );

void CypherLog_Emitf( const level_t log_level, const channel_t channel,
                const char *file, const char *function, const common::com_i32 line,
                const char *format, ... );

void CypherLog_Emitfv( const level_t log_level, const channel_t channel,
                 const char *file, const char *function, const common::com_i32 line,
                 const char *format, va_list args );

}

/*
================
Log Macros

Capture source file, function and line while keeping call sites compact.
================
*/
#define CYPHER_LOG_IF_ENABLED( LOG_LEVEL, LOG_CHANNEL )                                                           \
	if ( cypher::engine::log::CypherLog_LevelEnabled( ( LOG_LEVEL ), ( LOG_CHANNEL ) ) )

#define CYPHER_LOG( LOG_LEVEL, LOG_CHANNEL, LOG_MESSAGE )                                                         \
    CYPHER_LOGF( ( LOG_LEVEL ), ( LOG_CHANNEL ), "%s", ( LOG_MESSAGE ) )

#define CYPHER_LOGF( LOG_LEVEL, LOG_CHANNEL, LOG_FORMAT, ... )                                                    \
    do {                                                                                                        \
        if ( cypher::engine::log::CypherLog_LevelEnabled( ( LOG_LEVEL), ( LOG_CHANNEL ) ) ) {                          \
            cypher::engine::log::CypherLog_Emitf(                                                                      \
                ( LOG_LEVEL ), ( LOG_CHANNEL ), __FILE__, __func__, __LINE__, ( LOG_FORMAT )                     \
				__VA_OPT__( , ) __VA_ARGS__ );                                                             \
        }                                                                                                       \
    } while ( false );

#define CYPHER_LOGF_IF( LOG_CONDITION, LOG_LEVEL, LOG_CHANNEL, LOG_FORMAT, ... )                                  \
    do {                                                                                                        \
        if ( ( LOG_CONDITION ) && cypher::engine::log::CypherLog_LevelEnabled( ( LOG_LEVEL ), (LOG_CHANNEL ) ) ) {    \
            cypher::engine::log::CypherLog_Emitf(                                                                      \
                ( LOG_LEVEL ), ( LOG_CHANNEL ), __FILE__, __func__, __LINE__, ( LOG_FORMAT )                    \
				__VA_OPT__( , ) __VA_ARGS__ );                                                             \
        }                                                                                                       \
    } while ( false );

#define CYPHER_LOG_TRACE( LOG_CHANNEL, LOG_FORMAT, ... )          CYPHER_LOGF( cypher::engine::log::level_t::TRACE, ( LOG_CHANNEL ), ( LOG_FORMAT ) __VA_OPT__( , ) __VA_ARGS__ )

#define CYPHER_LOG_DEBUG( LOG_CHANNEL, LOG_FORMAT, ... )          CYPHER_LOGF( cypher::engine::log::level_t::DEBUG, ( LOG_CHANNEL ), ( LOG_FORMAT ) __VA_OPT__( , ) __VA_ARGS__ )

#define CYPHER_LOG_INFO( LOG_CHANNEL, LOG_FORMAT, ... )           CYPHER_LOGF( cypher::engine::log::level_t::INFO, ( LOG_CHANNEL ), ( LOG_FORMAT ) __VA_OPT__( , ) __VA_ARGS__ )

#define CYPHER_LOG_WARNING( LOG_CHANNEL, LOG_FORMAT, ... )        CYPHER_LOGF( cypher::engine::log::level_t::WARNING, ( LOG_CHANNEL ), ( LOG_FORMAT ) __VA_OPT__( , ) __VA_ARGS__ )

#define CYPHER_LOG_ERROR( LOG_CHANNEL, LOG_FORMAT, ... )          CYPHER_LOGF( cypher::engine::log::level_t::ERROR, ( LOG_CHANNEL ), ( LOG_FORMAT ) __VA_OPT__( , ) __VA_ARGS__ )

#define CYPHER_LOG_FATAL( LOG_CHANNEL, LOG_FORMAT, ... )          CYPHER_LOGF( cypher::engine::log::level_t::FATAL, ( LOG_CHANNEL ), ( LOG_FORMAT ) __VA_OPT__( , ) __VA_ARGS__ )

#define CYPHER_LOG_INFO_MESSAGE( LOG_CHANNEL, LOG_MESSAGE )       CYPHER_LOG( cypher::engine::log::level_t::INFO, ( LOG_CHANNEL ), ( LOG_MESSAGE ) )

#define CYPHER_LOG_WARNING_MESSAGE( LOG_CHANNEL, LOG_MESSAGE )    CYPHER_LOG( cypher::engine::log::level_t::WARNING, ( LOG_CHANNEL ), ( LOG_MESSAGE ) )

#define CYPHER_LOG_ERROR_MESSAGE( LOG_CHANNEL, LOG_MESSAGE )      CYPHER_LOG( cypher::engine::log::level_t::ERROR, ( LOG_CHANNEL ), ( LOG_MESSAGE ) )

#define CYPHER_LOG_CHECK( CONDITION, LOG_CHANNEL, LOG_FORMAT, ... )                                                                   \
    do {                                                                                                                            \
        if ( !CONDITION ) {                                                                                                         \
            CYPHER_LOG_ERROR( ( LOG_CHANNEL ), ( LOG_FORMAT ) __VA_OPT__( , ) __VA_ARGS__ );                                         \
        }                                                                                                                           \
    } while ( false );

#ifndef CYPHER_ENGINE_LOG_FORMAT_H
#define CYPHER_ENGINE_LOG_FORMAT_H

#pragma once

#include "CypherEngine/CypherLog/CypherLog_Error.h"
#include "CypherEngine/CypherLog/CypherLog_Types.h"

namespace cypher::engine::log
{

/*
================
Formatting functions

Used for formatting properly all of the logging system
================
*/
log_error_t CypherLog_FormatRecord(
    const record_t &record,
    const sink_config_t &pSinkConfig,
    const config_t &config,
    char *bufferOut,
    common::usize nOutBufferSize );

log_error_t CypherLog_FormatDetailed(
    const record_t &record,
    const sink_config_t &pSinkConfig,
    const config_t &config,
    char *bufferOut,
    const common::usize nOutBufferSize );

log_error_t CypherLog_FormatCompact(
    const record_t &record,
    const sink_config_t &pSinkConfig,
    char *bufferOut,
    const common::usize nOutBufferSize );

bool CypherLog_FormatTimestamp( const record_t &record, char *bufferOut, const common::usize nOutBufferSize );

const char *CypherLog_LevelColor( const level_t level );

}       // namespace cypher::engine::log

#endif // CYPHER_ENGINE_LOG_FORMAT_H

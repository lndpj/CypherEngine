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
error_code_t CypherLog_FormatRecord(
    const record_t &record,
    const sink_config_t &sink_config,
    const config_t &config,
    char *out_buffer,
    common::usize out_buffer_size );

error_code_t CypherLog_FormatDetailed(
    const record_t &record,
    const sink_config_t &sink_config,
    const config_t &config,
    char *out_buffer,
    const common::usize out_buffer_size );

error_code_t CypherLog_FormatCompact(
    const record_t &record,
    const sink_config_t &sink_config,
    char *out_buffer,
    const common::usize out_buffer_size );

bool CypherLog_FormatTimestamp( const record_t &record, char *out_buffer, const common::usize out_buffer_size );

const char *CypherLog_LevelColor( const level_t level );

}       // namespace cypher::engine::log

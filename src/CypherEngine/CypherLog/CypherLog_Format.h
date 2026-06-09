#pragma once

#include "CypherEngine/CypherLog/CypherLog_Error.h"
#include "CypherEngine/CypherLog/CypherLog_Types.h"

namespace cypher::engine::log
{

/*
================
CypherLog_FormatRecord

Formats a log record for one sink without writing it anywhere.
================
*/
error_code_t CypherLog_FormatRecord(
    const record_t &record,
    const sink_config_t &sink_config,
    const config_t &config,
    char *out_buffer,
    common::usize out_buffer_size );
}       // namespace cypher::engine::log

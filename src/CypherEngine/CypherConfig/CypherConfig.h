
#pragma once

#include "CypherEngine/CypherConfig/CypherConfig_Error.h"

namespace cypher::engine::cfg {

/*
================
Config Limits
================
*/
constexpr common::u32 CYPHER_CONFIG_MAX_LINE_LENGTH = 1024u;
constexpr common::u32 CYPHER_CONFIG_MAX_PATH_LENGTH = 260u;
constexpr common::u64 CYPHER_CONFIG_MAX_FILE_SIZE = 64u * 1024;

/*
================
Config API

Loads cfg files and routes each line into cvars or command execution.
================
*/
cfg_error_t CypherConfig_Init();
cfg_error_t CypherConfig_Shutdown();

cfg_error_t CypherConfig_LoadFile( const char *path, bool required = false );
cfg_error_t CypherConfig_LoadDefault();
cfg_error_t CypherConfig_LoadAutoexec();

cfg_error_t CypherConfig_ExecuteLine( const char *command_line );

}

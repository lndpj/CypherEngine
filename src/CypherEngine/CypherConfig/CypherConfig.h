
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
cypher_config_error_code_t CypherConfig_Init();
cypher_config_error_code_t CypherConfig_Shutdown();

cypher_config_error_code_t CypherConfig_LoadFile( const char *path, bool required = false );
cypher_config_error_code_t CypherConfig_LoadDefault();
cypher_config_error_code_t CypherConfig_LoadAutoexec();

cypher_config_error_code_t CypherConfig_ExecuteLine( const char *command_line );

}

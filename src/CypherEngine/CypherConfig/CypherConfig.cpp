/*======================================================================
   File: CypherConfig.cpp
   Project: CypherEngine
   Author: ksiric <email@example.com>
   Created: 2026-04-24 15:56:36
   Last Modified by: ksiric
   Last Modified: 2026-06-05 11:54:41
   ---------------------------------------------------------------------
   Description:

   ---------------------------------------------------------------------
   License:
   Company:
   Version: 0.1.0
 ======================================================================
																	   */
#include "CypherEngine/CypherConfig/CypherConfig.h"
#include "CypherEngine/CypherCommand/CypherCommand.h"
#include "CypherEngine/CypherCVar/CypherCVar.h"
#include "CypherEngine/CypherFileSystem/CypherFileSystem.h"

#include <cctype>      // std::isspace while parsing cfg lines.
#include <cstring>     // strlen / strcmp / strncpy for line parsing.

namespace cypher::engine::cfg {

/*
================
Config Runtime State
================
*/
struct cypher_config_runtime_state_t {
	bool initialized{ false };
};

cypher_config_runtime_state_t g_cfg_runtime_state;

constexpr const char *CYPHER_CONFIG_DEFAULT_PATH  = "config/default.cfg";
constexpr const char *CYPHER_CONFIG_AUTOEXEC_PATH = "config/autoexec.cfg";

/*
================
CypherConfig_Init
================
*/
cypher_config_error_code_t CypherConfig_Init() {
	if ( g_cfg_runtime_state.initialized ) {
		return cypher_config_error_code_t::ERR_IS_INIT;
	}
	g_cfg_runtime_state = {};
	g_cfg_runtime_state.initialized = true;
	return cypher_config_error_code_t::OK;
}

/*
================
CypherConfig_Shutdown
================
*/
cypher_config_error_code_t CypherConfig_Shutdown() {
	if ( !g_cfg_runtime_state.initialized ) {
		return cypher_config_error_code_t::ERR_NOT_INIT;
	}
	g_cfg_runtime_state = {};
	return cypher_config_error_code_t::OK;
}

/*
================
CypherConfig_LoadFile

Loads a cfg file through FS and executes it one line at a time.
================
*/
cypher_config_error_code_t CypherConfig_LoadFile( const char *path, const bool required ) {
	if ( path == nullptr || path[0] == '\0' ) {
		return cypher_config_error_code_t::ERR_INVALID_PATH;
	}
	if ( !g_cfg_runtime_state.initialized ) {
		return cypher_config_error_code_t::ERR_NOT_INIT;
	}
    
    fs::cypher_filesystem_file_t file{};
    
    const fs::cypher_filesystem_error_code_t open_result = fs::CypherFileSystem_Open( path, fs::cypher_filesystem_open_mode_t::READ_TEXT, file );
    if ( open_result != fs::cypher_filesystem_error_code_t::OK ) {
        return required ? cypher_config_error_code_t::ERR_FILE_OPEN_FAILED : cypher_config_error_code_t::OK;
    }
    
    if ( file.size == 0u ) {
        fs::CypherFileSystem_Close( file );
        return cypher_config_error_code_t::OK;
    }
    
    if ( file.size >= CYPHER_CONFIG_MAX_FILE_SIZE ) {
        fs::CypherFileSystem_Close( file );
        return cypher_config_error_code_t::ERR_IO_ERROR;
    }   
    
    char buffer[CYPHER_CONFIG_MAX_FILE_SIZE]{};
    common::u64 bytes_read{};
    
    const fs::cypher_filesystem_error_code_t read_result = fs::CypherFileSystem_Read( file, buffer, file.size, bytes_read );
    const fs::cypher_filesystem_error_code_t close_result = fs::CypherFileSystem_Close( file );
    
    if ( read_result != fs::cypher_filesystem_error_code_t::OK ) {
        return cypher_config_error_code_t::ERR_IO_ERROR;          
    }
    
    if ( close_result != fs::cypher_filesystem_error_code_t::OK ) {
        return cypher_config_error_code_t::ERR_IO_ERROR;          
    }
    
    buffer[bytes_read] = '\0';
    cypher_config_error_code_t result = cypher_config_error_code_t::OK;
    
    char *line_start = buffer;
    
    while ( *line_start != '\0' ) {
        char *line_end = line_start;
        
        while ( *line_end != '\0' && *line_end != '\n' && *line_end != '\r' ) {
            ++line_end;
        }
        
        char save_line_end = *line_end;
        *line_end = '\0';
        
        const cypher_config_error_code_t line_result = CypherConfig_ExecuteLine( line_start );
        
        if ( line_result != cypher_config_error_code_t::OK && result == cypher_config_error_code_t::OK ) {
            result = line_result;
        }   
        
        if ( save_line_end == '\0' ) {
            break;   
        }
        
        line_start = line_end + 1;
        
        // Skip the second byte of Windows CRLF line endings.
        if ( save_line_end == '\r' && *line_start == '\n' ) {
            ++line_start;
        }
    }
    
    return result;
}

/*
================
CypherConfig_LoadDefault
================
*/
cypher_config_error_code_t CypherConfig_LoadDefault() {
	if ( !g_cfg_runtime_state.initialized ) {
		return cypher_config_error_code_t::ERR_NOT_INIT;
	}
	return CypherConfig_LoadFile( CYPHER_CONFIG_DEFAULT_PATH, true );
}

/*
================
CypherConfig_LoadAutoexec
================
*/
cypher_config_error_code_t CypherConfig_LoadAutoexec() {
	if ( !g_cfg_runtime_state.initialized ) {
		return cypher_config_error_code_t::ERR_NOT_INIT;
	}
	return CypherConfig_LoadFile( CYPHER_CONFIG_AUTOEXEC_PATH, false );
}

/*
================
CypherConfig_ExecuteLine

Executes one trimmed cfg line: exec, set/seta, or regular command.
================
*/
cypher_config_error_code_t CypherConfig_ExecuteLine( const char *command_line ) {
	if ( !g_cfg_runtime_state.initialized ) {
		return cypher_config_error_code_t::ERR_INVALID_LINE;
	}
    
    if ( command_line == nullptr ) {
        return cypher_config_error_code_t::ERR_INVALID_LINE;
    }
	char line[CYPHER_CONFIG_MAX_LINE_LENGTH] {};
	std::strncpy( line, command_line, sizeof( line ) - 1 );

	char *cursor = line;
	while ( std::isspace( static_cast<unsigned char>( *cursor ) ) ) {
		++cursor;
	}
	if ( *cursor == '\0' || *cursor == '\n' || *cursor == '\r' ) {
		return cypher_config_error_code_t::OK;
	}
	bool in_quotes = false;
	for ( char *it = cursor; *it != '\0'; ++it ) {
		// Comments are ignored unless they are inside quoted values.
		if ( *it == '"' ) {
			in_quotes = !in_quotes;
		}
		if ( !in_quotes && it[0] == '/' && it[1] == '/' ) {
			*it = '\0';
			break;
		}
		if ( !in_quotes && *it == '#' ) {
			*it = '\0';
			break;
		}
	}
	char *end = cursor + std::strlen( cursor );
	while ( end > cursor && std::isspace( static_cast<unsigned char>( end[-1] ) ) ) {
		--end;
	}
	*end = '\0';
	if ( *cursor == '\0' ) {
		return cypher_config_error_code_t::OK;
	}
	char command[32]{};
	common::u32 i = 0;
	while ( *cursor != '\0' && !std::isspace( static_cast<unsigned char>( *cursor ) ) && ( i + 1u ) < sizeof( command ) ) {
		command[i++] = *cursor++;
	}
	command[i] = '\0';
	if ( std::strcmp( command, "exec" ) == 0 ) {
		while ( std::isspace( static_cast<unsigned char>( *cursor ) ) ) {
			++cursor;
		}
		if ( *cursor == '\0' ) {
			return cypher_config_error_code_t::ERR_PARSE_FAILED;
		}
		char exec_path[CYPHER_CONFIG_MAX_PATH_LENGTH]{};
		i = 0;
		if ( *cursor == '"' ) {
			++cursor;
			while ( cursor[i] != '\0' && cursor[i] != '"' && ( i + 1u ) < sizeof( exec_path ) ) {
				exec_path[i] = cursor[i];
				++i;
			}
			if ( cursor[i] != '"' ) {
				return cypher_config_error_code_t::ERR_PARSE_FAILED;
			}
		} else {
			while ( cursor[i] != '\0' && !std::isspace( static_cast<unsigned char>( cursor[i] ) ) && ( i + 1u ) < sizeof( exec_path ) ) {
				exec_path[i] = cursor[i];
				++i;
			}
		}
		exec_path[i] = '\0';
		return CypherConfig_LoadFile( exec_path, false );
	}
	if ( std::strcmp( command, "set" ) == 0 || std::strcmp( command, "seta" ) == 0 ) {
		while ( std::isspace( static_cast<unsigned char>( *cursor ) ) ) {
			++cursor;
		}
		if ( *cursor == '\0' ) {
			return cypher_config_error_code_t::ERR_PARSE_FAILED;
		}
		char cvar_name[256]{};
		i = 0;
		while ( *cursor != '\0' && !std::isspace( static_cast<unsigned char>( *cursor ) ) && ( i + 1u ) < sizeof( cvar_name ) ) {
			cvar_name[i++] = *cursor++;
		}
		cvar_name[i] = '\0';

		if ( cvar_name[0] == '\0' ) {
			return cypher_config_error_code_t::ERR_PARSE_FAILED;
		}
		while ( std::isspace( static_cast<unsigned char>( *cursor ) ) ) {
			++cursor;
		}
		if ( *cursor == '\0' ) {
			return cypher_config_error_code_t::ERR_PARSE_FAILED;
		}
		char cvar_value[CYPHER_CONFIG_MAX_LINE_LENGTH]{};
		i = 0;
		if ( *cursor == '"' ) {
			++cursor;
			while ( cursor[i] != '\0' && cursor[i] != '"' && ( i + 1u ) < sizeof( cvar_value ) ) {
				cvar_value[i] = cursor[i];
				++i;
			}
			if ( cursor[i] != '"' ) {
				return cypher_config_error_code_t::ERR_PARSE_FAILED;
			}
		} else {
			while ( cursor[i] != '\0' && !std::isspace( static_cast<unsigned char>( cursor[i] ) ) && ( i + 1u ) < sizeof( cvar_value ) ) {
				cvar_value[i] = cursor[i];
				++i;
			}
		}
		cvar_value[i] = '\0';
		if ( cvar_value[0] == '\0' ) {
			return cypher_config_error_code_t::ERR_PARSE_FAILED;
		}
		if ( cvar::CypherCVar_Set( cvar_name, cvar_value ) != cvar::cypher_cvar_error_code_t::OK ) {
			return cypher_config_error_code_t::ERR_PARSE_FAILED;
		}
		return cypher_config_error_code_t::OK;
	}
	if ( cmd::CypherCommand_Execute( line ) != cmd::cypher_command_error_code_t::OK ) {
		return cypher_config_error_code_t::ERR_COMMAND_FAILED;
	}
	return cypher_config_error_code_t::OK;
}

} // namespace cypher::engine::cfg

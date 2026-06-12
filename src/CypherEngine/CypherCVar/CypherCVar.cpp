/*======================================================================
   File: CypherCVar.cpp
   Project: CypherEngine
   Author: ksiric <email@example.com>
   Created: 2026-04-22 21:04:15
   Last Modified by: ksiric
   Last Modified: 2026-06-12 13:35:20
   ---------------------------------------------------------------------
   Description:

   ---------------------------------------------------------------------
   License:
   Company:
   Version: 0.1.0
 ======================================================================
																	   */
#include "CypherEngine/CypherCVar/CypherCVar.h"
#include "CypherEngine/CypherCVar/CypherCVar_Error.h"
#include "CypherEngine/CypherLog/CypherLog.h"

#include <cctype>      // std::tolower for bool parsing.
#include <cstdlib>     // atoi / atof numeric conversion.
#include <cstring>     // strcmp / strncpy for cvar storage.

namespace cypher::engine::cvar {

registry_t g_cvar_registry;

/*
================
CypherCVar_Init
================
*/
cvar_error_t CypherCVar_Init() {
	if ( g_cvar_registry.initialized ) {
		LOG_WARNING( log::channel_t::CVAR, "cvar system init requested while already initialized." );
		return cvar_error_t::ERR_IS_INIT;
	}

	g_cvar_registry = {};
	g_cvar_registry.initialized = true;

	LOG_INFO( log::channel_t::CVAR, "cvar system initialized." );

	return cvar_error_t::OK;
}

/*
================
CypherCVar_ParseBool

Accepts common console-friendly true/false strings.
================
*/
bool CypherCVar_ParseBool( const char *value ) {
	if ( value == nullptr || value[0] == '\0' ) {
		return false;
	}

	char lower[32]{};
	common::u32 i;

	for ( i = 0; i < 31 && value[i] != '\0'; i++ ) {
    lower[i] = static_cast<char>( std::tolower( static_cast<unsigned char>( value[i] ) ) );
	}
	lower[i] = '\0';

	if ( std::strcmp( lower, "0" ) == 0 || std::strcmp( lower, "false" ) == 0 || std::strcmp( lower, "off" ) == 0 || std::strcmp( lower, "no" ) == 0 ) {
		return false;
	}

	if ( std::strcmp( lower, "1" ) == 0 || std::strcmp( lower, "true" ) == 0 || std::strcmp( lower, "on" ) == 0 || std::strcmp( lower, "yes" ) == 0 ) {
		return true;
	}

	return ( std::atoi( lower ) != 0 );
}

/*
================
CypherCVar_Register

Adds a cvar with default string and cached numeric views.
================
*/
cvar_error_t CypherCVar_Register( const char *name, const char *default_value, flags_t flags ) {
	if ( !g_cvar_registry.initialized ) {
		LOG_ERROR( log::channel_t::CVAR, "cvar register failed for '%s': cvar system is not initialized.", name ? name : "<null>" );
		return cvar_error_t::ERR_NOT_INIT;
	}

	if ( name == nullptr || name[0] == '\0' ) {
		LOG_ERROR( log::channel_t::CVAR, "cvar register failed: invalid cvar name." );
		return cvar_error_t::ERR_INVALID_CVAR;
	}

	if ( default_value == nullptr || default_value[0] == '\0' ) {
		LOG_ERROR( log::channel_t::CVAR, "cvar register failed for '%s': invalid default value.", name );
		return cvar_error_t::ERR_INVALID_DEFAULT_VALUE;
	}

	common::u32 flags_bits = static_cast<common::u32>( flags );

	if ( ( flags_bits & CYPHER_CVAR_MODIFIED ) != 0u ) {
		LOG_ERROR( log::channel_t::CVAR, "cvar register failed for '%s': registration passed runtime modified flag.", name );
		return cvar_error_t::ERR_INVALID_FLAG;
	}

	// CYPHER_CVAR_MODIFIED is runtime-owned; registration only accepts authoring flags.
	if ( ( flags_bits & ~CYPHER_CVAR_REGISTER_ALLOWED_FLAGS ) != 0 ) {
		LOG_ERROR( log::channel_t::CVAR, "cvar register failed for '%s': invalid flags 0x%x.", name, flags_bits );
		return cvar_error_t::ERR_INVALID_FLAG;
	}

	const cvar_t *cvar = CypherCVar_Find( name );

	if ( cvar != nullptr ) {
		LOG_WARNING( log::channel_t::CVAR, "cvar register skipped: '%s' already exists.", name );
		return cvar_error_t::ERR_CVAR_ALREADY_EXISTS;
	}

	if ( g_cvar_registry.cvar_count >= CYPHER_CVAR_MAX_CVARS ) {
		LOG_ERROR( log::channel_t::CVAR, "cvar register failed for '%s': registry full (%u).", name, CYPHER_CVAR_MAX_CVARS );
		return cvar_error_t::ERR_REGISTRY_FULL;
	}

	cvar_t &entry = g_cvar_registry.cvars[g_cvar_registry.cvar_count];

	entry.name = name;
	std::strncpy( entry.value_string, default_value, sizeof( entry.value_string ) - 1 );
	std::strncpy( entry.default_string, default_value, sizeof( entry.default_string ) - 1 );
	entry.value_string[sizeof( entry.value_string ) - 1] = '\0';
	entry.default_string[sizeof( entry.default_string ) - 1] = '\0';
	entry.value_int = std::atoi( default_value );
	entry.value_float = std::atof( default_value );
	entry.flags = flags;
	entry.value_bool = CypherCVar_ParseBool( entry.value_string );
	g_cvar_registry.cvar_count++;

	LOG_DEBUG( log::channel_t::CVAR, "registered cvar '%s' default='%s' flags=0x%x.", name, default_value, flags_bits );

	return cvar_error_t::OK;
}

/*
================
CypherCVar_Set

Changes a cvar string and updates its cached int/float/bool values.
================
*/
cvar_error_t CypherCVar_Set( const char *name, const char *value ) {
	if ( !g_cvar_registry.initialized ) {
		LOG_ERROR( log::channel_t::CVAR, "cvar set failed for '%s': cvar system is not initialized.", name ? name : "<null>" );
		return cvar_error_t::ERR_NOT_INIT;
	}

	if ( name == nullptr || name[0] == '\0' ) {
        LOG_ERROR( log::channel_t::CVAR, "cvar set failed: invalid cvar name." );
        return cvar_error_t::ERR_INVALID_CVAR;
	}
    
    if ( value == nullptr ) {
        LOG_ERROR( log::channel_t::CVAR, "cvar set failed for '%s': value is null.", name );
        return cvar_error_t::ERR_INVALID_CVAR;
        
    }
    
    cvar_t *target = nullptr;

    // Linear for now; later this can become a hash table without changing API.
    for ( common::u32 i = 0; i < g_cvar_registry.cvar_count; ++i ) {
        if ( std::strcmp( g_cvar_registry.cvars[i].name, name ) == 0 ) {
            target = &g_cvar_registry.cvars[i];
            break;
        }
    }
    if ( target == nullptr ) {
        LOG_WARNING( log::channel_t::CVAR, "cvar set failed: '%s' not found.", name );
        return cvar_error_t::ERR_CVAR_NOT_FOUND;
    } 
    if ( ( static_cast<common::u32>( target->flags ) & CYPHER_CVAR_READONLY ) != 0 ) {
        LOG_WARNING( log::channel_t::CVAR, "cvar set blocked: '%s' is read-only.", name );
        return cvar_error_t::ERR_READONLY;
    }
    
    // Cheat protection will later be controlled by server/game authority.
    const bool cheats_enabled = false;
    if ( ( static_cast<common::u32>( target->flags ) & CYPHER_CVAR_CHEAT ) != 0 && !cheats_enabled ) {
        LOG_WARNING( log::channel_t::CVAR, "cvar set blocked: '%s' is cheat-protected.", name );
        return cvar_error_t::ERR_CHEAT_PROTECTED;
    }
    
    std::strncpy( target->value_string, value, sizeof( target->value_string ) - 1 );
    target->value_string[sizeof( target->value_string ) - 1u] = '\0';
    
    target->value_int = static_cast<common::u32>( std::atoi( target->value_string ) );
    target->value_float = static_cast<common::f32>( std::atof( target->value_string ) );
    target->value_bool = CypherCVar_ParseBool( target->value_string );
    
    target->flags = static_cast<flags_t>( static_cast<common::u32>( target->flags ) | CYPHER_CVAR_MODIFIED );

    LOG_DEBUG( log::channel_t::CVAR, "cvar '%s' set to '%s'.", name, target->value_string );
    
    return cvar_error_t::OK;
}

/*
================
CypherCVar_Shutdown
================
*/
cvar_error_t CypherCVar_Shutdown() {
    if ( !g_cvar_registry.initialized ) {
        LOG_WARNING( log::channel_t::CVAR, "cvar system shutdown requested while not initialized." );
        return cvar_error_t::ERR_NOT_INIT;
    }
    LOG_INFO( log::channel_t::CVAR, "cvar system shutdown: cvars=%u.", g_cvar_registry.cvar_count );
    g_cvar_registry = {};
    
    return cvar_error_t::OK;
}

/*
================
CypherCVar_Find
================
*/
const cvar_t *CypherCVar_Find( const char *name ) {
    if ( !g_cvar_registry.initialized ) {
        return nullptr;
    }
    
    if ( name == nullptr || name[0] == '\0' ) {
        return nullptr;
    }
    
    const cvar_t *cvar = nullptr;
    
    for ( common::u32 i = 0; i < g_cvar_registry.cvar_count; ++i ) {
        if ( g_cvar_registry.cvars[i].name == nullptr ) { 
            continue;
        }
        if ( std::strcmp( g_cvar_registry.cvars[i].name, name ) == 0 ) {
            cvar = &g_cvar_registry.cvars[i];
            return cvar;
        } 
    }
    
    return cvar;
}

/*
================
CypherCVar_GetString
================
*/
const char *CypherCVar_GetString( const char *name ) {
    if ( !g_cvar_registry.initialized ) {
        return nullptr;
    }
    
    if ( name == nullptr || name[0] == '\0' ) {
        return nullptr;
    }
    
    const cvar_t *cvar = CypherCVar_Find( name );
    
    if ( cvar == nullptr ) {
        return "";
    } 
    
    return cvar->value_string;
}

/*
================
CypherCVar_GetInt
================
*/
common::u32 CypherCVar_GetInt( const char *name ) {
    if ( !g_cvar_registry.initialized ) {
        return (common::u32)0u;
    }
    
    if ( name == nullptr || name[0] == '\0' ) {
        return (common::u32)0u;
    }
    
    const cvar_t *cvar = CypherCVar_Find( name );
    
    if ( cvar == nullptr ) {
        return (common::u32)0u;
    }
    
    return cvar->value_int;
}

/*
================
CypherCVar_GetFloat
================
*/
common::f32 CypherCVar_GetFloat( const char *name ) {
    if ( !g_cvar_registry.initialized ) {
        return (common::f32)0.0f;
    }
    
    if ( name == nullptr || name[0] == '\0' ) {
        return (common::f32)0.0f;
    }
    
    const cvar_t *cvar = CypherCVar_Find( name );
    
    if ( cvar == nullptr ) {
        return (common::f32)0.0f;
    }
    
    return cvar->value_float;
}

/*
================
CypherCVar_GetBool
================
*/
bool CypherCVar_GetBool( const char *name ) {
    if ( !g_cvar_registry.initialized ) {
        return false;
    }
    
    if ( name == nullptr || name[0] == '\0' ) {
        return false;
    }
    
    const cvar_t *cvar = CypherCVar_Find( name );
    
    if ( cvar == nullptr ) {
        return false;
    }
    
    return cvar->value_bool;
}

} // namespace cypher::engine::cvar

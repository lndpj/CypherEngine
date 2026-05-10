/*======================================================================
   File: cvar_main.cpp
   Project: REAP
   Author: ksiric <email@example.com>
   Created: 2026-04-22 21:04:15
   Last Modified by: ksiric
   Last Modified: 2026-05-05 00:16:30
   ---------------------------------------------------------------------
   Description:

   ---------------------------------------------------------------------
   License:
   Company:
   Version: 0.1.0
 ======================================================================
																	   */
#include "rengine/cvar/cvar_main.h"
#include "rengine/cvar/cvar_error.h"
#include "rengine/rcommon/com_print.h"

#include <cctype>      // std::tolower for bool parsing.
#include <cstdlib>     // atoi / atof numeric conversion.
#include <cstring>     // strcmp / strncpy for cvar storage.

namespace reap::rengine::cvar {

cvar_registry_t g_cvar_registry;

/*
================
Cvar_Init
================
*/
cvar_error_code_t Cvar_Init() {
	if ( g_cvar_registry.initialized ) {
		rcommon::Com_Printf( "Cvar_Init: %s: %s\n", Cvar_ErrorName( cvar_error_code_t::ERR_IS_INIT ), Cvar_ErrorDesc( cvar_error_code_t::ERR_IS_INIT ) );
		return cvar_error_code_t::ERR_IS_INIT;
	}

	g_cvar_registry = {};
	g_cvar_registry.initialized = true;

	return cvar_error_code_t::OK;
}

/*
================
Cvar_ParseBool

Accepts common console-friendly true/false strings.
================
*/
bool Cvar_ParseBool( const char *value ) {
	if ( value == nullptr || value[0] == '\0' ) {
		return false;
	}

	char lower[32]{};
	rcommon::u32 i;

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
Cvar_Register

Adds a cvar with default string and cached numeric views.
================
*/
cvar_error_code_t Cvar_Register( const char *name, const char *default_value, cvar_flags_t flags ) {
	if ( !g_cvar_registry.initialized ) {
		rcommon::Com_Printf( "Cvar_Register: %s: %s\n", Cvar_ErrorName( cvar_error_code_t::ERR_NOT_INIT ), Cvar_ErrorDesc( cvar_error_code_t::ERR_NOT_INIT ) );
		return cvar_error_code_t::ERR_NOT_INIT;
	}

	if ( name == nullptr || name[0] == '\0' ) {
		rcommon::Com_Printf( "Cvar_Register: %s: %s\n", Cvar_ErrorName( cvar_error_code_t::ERR_INVALID_CVAR ), Cvar_ErrorDesc( cvar_error_code_t::ERR_INVALID_CVAR ) );
		return cvar_error_code_t::ERR_INVALID_CVAR;
	}

	if ( default_value == nullptr || default_value[0] == '\0' ) {
		rcommon::Com_Printf( "Cvar_Register: %s: %s\n", Cvar_ErrorName( cvar_error_code_t::ERR_INVALID_DEFAULT_VALUE ), Cvar_ErrorDesc( cvar_error_code_t::ERR_INVALID_DEFAULT_VALUE ) );
		return cvar_error_code_t::ERR_INVALID_DEFAULT_VALUE;
	}

	rcommon::u32 flags_bits = static_cast<rcommon::u32>( flags );

	if ( ( flags_bits & CVAR_MODIFIED ) != 0u ) {
		rcommon::Com_Printf( "Cvar_Register: %s: %s\n", Cvar_ErrorName( cvar_error_code_t::ERR_INVALID_FLAG ), Cvar_ErrorDesc( cvar_error_code_t::ERR_INVALID_FLAG ) );
		return cvar_error_code_t::ERR_INVALID_FLAG;
	}

	// CVAR_MODIFIED is runtime-owned; registration only accepts authoring flags.
	if ( ( flags_bits & ~CVAR_REGISTER_ALLOWED_FLAGS ) != 0 ) {
		rcommon::Com_Printf( "Cvar_Register: %s: %s\n", Cvar_ErrorName( cvar_error_code_t::ERR_INVALID_FLAG ), Cvar_ErrorDesc( cvar_error_code_t::ERR_INVALID_FLAG ) );
		return cvar_error_code_t::ERR_INVALID_FLAG;
	}

	const cvar_t *cvar = Cvar_Find( name );

	if ( cvar != nullptr ) {
		rcommon::Com_Printf( "Cvar_Register: %s: %s\n", Cvar_ErrorName( cvar_error_code_t::ERR_CVAR_ALREADY_EXISTS ), Cvar_ErrorDesc( cvar_error_code_t::ERR_CVAR_ALREADY_EXISTS ) );
		return cvar_error_code_t::ERR_CVAR_ALREADY_EXISTS;
	}

	if ( g_cvar_registry.cvar_count >= CVAR_MAX_CVARS ) {
		rcommon::Com_Printf( "Cvar_Register: %s: %s\n", Cvar_ErrorName( cvar_error_code_t::ERR_REGISTRY_FULL ), Cvar_ErrorDesc( cvar_error_code_t::ERR_REGISTRY_FULL ) );
		return cvar_error_code_t::ERR_REGISTRY_FULL;
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
	entry.value_bool = Cvar_ParseBool( entry.value_string );
	g_cvar_registry.cvar_count++;

	return cvar_error_code_t::OK;
}

/*
================
Cvar_Set

Changes a cvar string and updates its cached int/float/bool values.
================
*/
cvar_error_code_t Cvar_Set( const char *name, const char *value ) {
	if ( !g_cvar_registry.initialized ) {
		rcommon::Com_Printf( "Cvar_Set: %s: %s\n", Cvar_ErrorName( cvar_error_code_t::ERR_NOT_INIT ), Cvar_ErrorDesc( cvar_error_code_t::ERR_NOT_INIT ) );
		return cvar_error_code_t::ERR_NOT_INIT;
	}

	if ( name == nullptr || name[0] == '\0' ) {
        rcommon::Com_Printf( "Cvar_Set: %s: %s\n",
                             Cvar_ErrorName( cvar_error_code_t::ERR_INVALID_CVAR ),
                             Cvar_ErrorDesc( cvar_error_code_t::ERR_INVALID_CVAR ) );
        return cvar_error_code_t::ERR_INVALID_CVAR;
	}
    
    if ( value == nullptr ) {
        rcommon::Com_Printf( "Cvar_Set: %s: %s\n",
                             Cvar_ErrorName( cvar_error_code_t::ERR_INVALID_CVAR ),
                             Cvar_ErrorDesc( cvar_error_code_t::ERR_INVALID_CVAR ) );
        return cvar_error_code_t::ERR_INVALID_CVAR;
        
    }
    
    cvar_t *target = nullptr;

    // Linear for now; later this can become a hash table without changing API.
    for ( rcommon::u32 i = 0; i < g_cvar_registry.cvar_count; ++i ) {
        if ( std::strcmp( g_cvar_registry.cvars[i].name, name ) == 0 ) {
            target = &g_cvar_registry.cvars[i];
            break;
        }
    }
    if ( target == nullptr ) {
        rcommon::Com_Printf( "Cvar_Set: %s: %s\n",
                             Cvar_ErrorName( cvar_error_code_t::ERR_CVAR_NOT_FOUND ),
                             Cvar_ErrorDesc( cvar_error_code_t::ERR_CVAR_NOT_FOUND ) );
        return cvar_error_code_t::ERR_CVAR_NOT_FOUND;
    } 
    if ( ( static_cast<rcommon::u32>( target->flags ) & CVAR_READONLY ) != 0 ) {
        rcommon::Com_Printf( "Cvar_Set: %s: %s\n", Cvar_ErrorName( cvar_error_code_t::ERR_READONLY ), Cvar_ErrorDesc( cvar_error_code_t::ERR_READONLY ) );
        return cvar_error_code_t::ERR_READONLY;
    }
    
    // Cheat protection will later be controlled by server/game authority.
    const bool cheats_enabled = false;
    if ( ( static_cast<rcommon::u32>( target->flags ) & CVAR_CHEAT ) != 0 && !cheats_enabled ) {
        rcommon::Com_Printf( "Cvar_Set: %s: %s\n",
                             Cvar_ErrorName( cvar_error_code_t::ERR_CHEAT_PROTECTED ),
                             Cvar_ErrorDesc( cvar_error_code_t::ERR_CHEAT_PROTECTED ) );
        return cvar_error_code_t::ERR_CHEAT_PROTECTED;
    }
    
    std::strncpy( target->value_string, value, sizeof( target->value_string ) - 1 );
    target->value_string[sizeof( target->value_string ) - 1u] = '\0';
    
    target->value_int = static_cast<rcommon::u32>( std::atoi( target->value_string ) );
    target->value_float = static_cast<rcommon::f32>( std::atof( target->value_string ) );
    target->value_bool = Cvar_ParseBool( target->value_string );
    
    target->flags = static_cast<cvar_flags_t>( static_cast<rcommon::u32>( target->flags ) | CVAR_MODIFIED );
    
    return cvar_error_code_t::OK;
}

/*
================
Cvar_Shutdown
================
*/
cvar_error_code_t Cvar_Shutdown() {
    if ( !g_cvar_registry.initialized ) {
        rcommon::Com_Printf( "Cvar_Shutdown: %s: %s\n", Cvar_ErrorName( cvar_error_code_t::ERR_NOT_INIT ), Cvar_ErrorDesc( cvar_error_code_t::ERR_NOT_INIT ) );
        return cvar_error_code_t::ERR_NOT_INIT;
    }
    g_cvar_registry = {};
    
    return cvar_error_code_t::OK;
}

/*
================
Cvar_Find
================
*/
const cvar_t *Cvar_Find( const char *name ) {
    if ( !g_cvar_registry.initialized ) {
        return nullptr;
    }
    
    if ( name == nullptr || name[0] == '\0' ) {
        return nullptr;
    }
    
    const cvar_t *cvar = nullptr;
    
    for ( rcommon::u32 i = 0; i < g_cvar_registry.cvar_count; ++i ) {
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
Cvar_GetString
================
*/
const char *Cvar_GetString( const char *name ) {
    if ( !g_cvar_registry.initialized ) {
        return nullptr;
    }
    
    if ( name == nullptr || name[0] == '\0' ) {
        return nullptr;
    }
    
    const cvar_t *cvar = Cvar_Find( name );
    
    if ( cvar == nullptr ) {
        return "";
    } 
    
    return cvar->value_string;
}

/*
================
Cvar_GetInt
================
*/
rcommon::u32 Cvar_GetInt( const char *name ) {
    if ( !g_cvar_registry.initialized ) {
        return (rcommon::u32)0u;
    }
    
    if ( name == nullptr || name[0] == '\0' ) {
        return (rcommon::u32)0u;
    }
    
    const cvar_t *cvar = Cvar_Find( name );
    
    if ( cvar == nullptr ) {
        return (rcommon::u32)0u;
    }
    
    return cvar->value_int;
}

/*
================
Cvar_GetFloat
================
*/
rcommon::f32 Cvar_GetFloat( const char *name ) {
    if ( !g_cvar_registry.initialized ) {
        return (rcommon::f32)0.0f;
    }
    
    if ( name == nullptr || name[0] == '\0' ) {
        return (rcommon::f32)0.0f;
    }
    
    const cvar_t *cvar = Cvar_Find( name );
    
    if ( cvar == nullptr ) {
        return (rcommon::f32)0.0f;
    }
    
    return cvar->value_float;
}

/*
================
Cvar_GetBool
================
*/
bool Cvar_GetBool( const char *name ) {
    if ( !g_cvar_registry.initialized ) {
        return false;
    }
    
    if ( name == nullptr || name[0] == '\0' ) {
        return false;
    }
    
    const cvar_t *cvar = Cvar_Find( name );
    
    if ( cvar == nullptr ) {
        return false;
    }
    
    return cvar->value_bool;
}

} // namespace reap::rengine::cvar

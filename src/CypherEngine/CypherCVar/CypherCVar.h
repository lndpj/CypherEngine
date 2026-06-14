#ifndef CYPHER_ENGINE_CVAR_H
#define CYPHER_ENGINE_CVAR_H

#pragma once

#include "CypherEngine/CypherCVar/CypherCVar_Error.h"

#define CYPHER_CVAR_MAX_CVARS 256u

namespace cypher::engine::cvar {

/*
================
Cvar Types

Cvars are named runtime variables used by configs, console and engine systems.
================
*/
enum flags_t : common::u32 {
	CYPHER_CVAR_NONE = 0,
	CYPHER_CVAR_ARCHIVE = 1 << 0,
	CYPHER_CVAR_READONLY = 1 << 1,
	CYPHER_CVAR_CHEAT = 1 << 2,
	CYPHER_CVAR_DEV = 1 << 3,
	CYPHER_CVAR_MODIFIED = 1 << 4
};

struct cvar_t {
	const char *name;
	char value_string[256];
	char default_string[256];
	common::u32 value_int;
	common::f32 value_float;
	bool value_bool;
	flags_t flags;
};

struct registry_t {
	cvar_t cvars[CYPHER_CVAR_MAX_CVARS];
	common::u32 cvar_count;
	bool initialized;
};

constexpr common::u32 CYPHER_CVAR_REGISTER_ALLOWED_FLAGS =
	CYPHER_CVAR_ARCHIVE | CYPHER_CVAR_READONLY | CYPHER_CVAR_CHEAT | CYPHER_CVAR_DEV;

/*
================
Cvar API
================
*/
cvar_error_t CypherCVar_Init();

cvar_error_t CypherCVar_Register( const char *name, const char *default_value, flags_t flags );

cvar_error_t CypherCVar_Set( const char *name, const char *value );

cvar_error_t CypherCVar_Shutdown();

const cvar_t *CypherCVar_Find( const char *name );

const char *CypherCVar_GetString( const char *name );

common::u32 CypherCVar_GetInt( const char *name );

common::f32 CypherCVar_GetFloat( const char *name );

bool CypherCVar_GetBool( const char *name );

}

#endif // CYPHER_ENGINE_CVAR_H

#pragma once

#include "rengine/cvar/cvar_error.h"

#define CVAR_MAX_CVARS 256u

namespace reap::rengine::cvar {

/*
================
Cvar Types

Cvars are named runtime variables used by configs, console and engine systems.
================
*/
enum cvar_flags_t : rcommon::u32 {
	CVAR_NONE = 0,
	CVAR_ARCHIVE = 1 << 0,
	CVAR_READONLY = 1 << 1,
	CVAR_CHEAT = 1 << 2,
	CVAR_DEV = 1 << 3,
	CVAR_MODIFIED = 1 << 4
};

struct cvar_t {
	const char *name;
	char value_string[256];
	char default_string[256];
	rcommon::u32 value_int;
	rcommon::f32 value_float;
	bool value_bool;
	cvar_flags_t flags;
};

struct cvar_registry_t {
	cvar_t cvars[CVAR_MAX_CVARS];
	rcommon::u32 cvar_count;
	bool initialized;
};

constexpr rcommon::u32 CVAR_REGISTER_ALLOWED_FLAGS =
	CVAR_ARCHIVE | CVAR_READONLY | CVAR_CHEAT | CVAR_DEV;

/*
================
Cvar API
================
*/
cvar_error_code_t Cvar_Init();

cvar_error_code_t Cvar_Register( const char *name, const char *default_value, cvar_flags_t flags );

cvar_error_code_t Cvar_Set( const char *name, const char *value );

cvar_error_code_t Cvar_Shutdown();

const cvar_t *Cvar_Find( const char *name );

const char *Cvar_GetString( const char *name );

rcommon::u32 Cvar_GetInt( const char *name );

rcommon::f32 Cvar_GetFloat( const char *name );

bool Cvar_GetBool( const char *name );

}

#pragma once

#include "rengine/host/host_error.h"
#include "rengine/host/host_types.h"

namespace reap::rengine::host
{

/*
================
Host Initialization Steps

Kept separate so startup order stays readable and failure cleanup is explicit.
================
*/
void Host_PrepareStateForInit( host_state_t &host_state );

host_error_code_t Host_InitCoreEngineSystems( host_state_t &host_state );

host_error_code_t Host_MountFileSystem( void );

host_error_code_t Host_RegisterBuiltinCvars( void );

host_error_code_t Host_RegisterBuiltinCommands( host_state_t &host_state );

host_error_code_t Host_LoadStartupConfig( void );

host_error_code_t Host_ApplyCvarsToConfig( host_state_t &host_state );

host_error_code_t Host_CreateWindow( host_state_t &host_state );

host_error_code_t Host_InitRenderer( host_state_t &host_state );

host_error_code_t Host_FinishInit( host_state_t &host_state );

/*
================
Host Runtime API
================
*/
host_error_code_t Host_Init( host_state_t &host_state );

void Host_RequestShutdown( host_state_t &host_state );

void Host_Shutdown( host_state_t &host_state );

void Host_BeginFrame( host_state_t &host_state );

void Host_Update( host_state_t &host_state );

void Host_Render( host_state_t &host_state );

void Host_EndFrame( host_state_t &host_state );

bool Host_IsRunning( host_state_t &host_state );

}

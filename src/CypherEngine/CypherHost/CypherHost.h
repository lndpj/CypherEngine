#pragma once

#include "CypherEngine/CypherHost/CypherHost_Error.h"
#include "CypherEngine/CypherHost/CypherHost_Types.h"

namespace cypher::engine::host
{

/*
================
Host Initialization Steps

Kept separate so startup order stays readable and failure cleanup is explicit.
================
*/
void CypherHost_PrepareStateForInit( state_t &host_state );

error_code_t CypherHost_InitCoreEngineSystems( state_t &host_state );

error_code_t CypherHost_MountFileSystem( void );

error_code_t CypherHost_RegisterBuiltinCvars( void );

error_code_t CypherHost_RegisterBuiltinCommands( state_t &host_state );

error_code_t CypherHost_LoadStartupConfig( void );

error_code_t CypherHost_ApplyCvarsToConfig( state_t &host_state );

error_code_t CypherHost_CreateWindow( state_t &host_state );

error_code_t CypherHost_InitRenderer( state_t &host_state );

error_code_t CypherHost_FinishInit( state_t &host_state );

/*
================
Host Runtime API
================
*/
error_code_t CypherHost_Init( state_t &host_state );

void CypherHost_RequestShutdown( state_t &host_state );

void CypherHost_Shutdown( state_t &host_state );

void CypherHost_BeginFrame( state_t &host_state );

void CypherHost_Update( state_t &host_state );

void CypherHost_Render( state_t &host_state );

void CypherHost_EndFrame( state_t &host_state );

bool CypherHost_IsRunning( state_t &host_state );

}

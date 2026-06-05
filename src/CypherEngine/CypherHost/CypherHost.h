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
void CypherHost_PrepareStateForInit( cypher_host_state_t &host_state );

cypher_host_error_code_t CypherHost_InitCoreEngineSystems( cypher_host_state_t &host_state );

cypher_host_error_code_t CypherHost_MountFileSystem( void );

cypher_host_error_code_t CypherHost_RegisterBuiltinCvars( void );

cypher_host_error_code_t CypherHost_RegisterBuiltinCommands( cypher_host_state_t &host_state );

cypher_host_error_code_t CypherHost_LoadStartupConfig( void );

cypher_host_error_code_t CypherHost_ApplyCvarsToConfig( cypher_host_state_t &host_state );

cypher_host_error_code_t CypherHost_CreateWindow( cypher_host_state_t &host_state );

cypher_host_error_code_t CypherHost_InitRenderer( cypher_host_state_t &host_state );

cypher_host_error_code_t CypherHost_FinishInit( cypher_host_state_t &host_state );

/*
================
Host Runtime API
================
*/
cypher_host_error_code_t CypherHost_Init( cypher_host_state_t &host_state );

void CypherHost_RequestShutdown( cypher_host_state_t &host_state );

void CypherHost_Shutdown( cypher_host_state_t &host_state );

void CypherHost_BeginFrame( cypher_host_state_t &host_state );

void CypherHost_Update( cypher_host_state_t &host_state );

void CypherHost_Render( cypher_host_state_t &host_state );

void CypherHost_EndFrame( cypher_host_state_t &host_state );

bool CypherHost_IsRunning( cypher_host_state_t &host_state );

}

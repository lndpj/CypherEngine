/*======================================================================
   File: main.cpp
   Project: CypherEngine
   Author: ksiric <email@example.com>
   Created: 2026-04-18 23:12:08
   Last Modified by: ksiric
   Last Modified: 2026-05-01 21:30:06
   ---------------------------------------------------------------------
   Description:

   ---------------------------------------------------------------------
   License:
   Company:
   Version: 0.1.0
 ======================================================================
                                                                       */
#include "CypherEngine/CypherHost/CypherHost.h"

#include <cstdlib>     // EXIT_SUCCESS / EXIT_FAILURE.

namespace rr = cypher::engine;
namespace host = rr::host;

/*
================
main

Keeps the executable entry point thin; host owns engine startup, frame flow
and shutdown ordering.
================
*/
int main(int argc, char const *argv[])
{
    host::state_t  pHostState{};
    pHostState.config.argc = argc;
    pHostState.config.argv = argv;

    if ( host::CypherHost_Init( pHostState ) != host::host_error_t::OK ) {
        return ( EXIT_FAILURE );
    }

    while( host::CypherHost_IsRunning( pHostState ) ) {
        host::CypherHost_BeginFrame( pHostState );
        host::CypherHost_Update( pHostState );
        host::CypherHost_Render( pHostState );
        host::CypherHost_EndFrame( pHostState );
    }

    host::CypherHost_Shutdown( pHostState );

    return ( EXIT_SUCCESS );
}

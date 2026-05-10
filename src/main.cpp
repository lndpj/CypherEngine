/*======================================================================
   File: main.cpp
   Project: REAP
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
#include "rengine/host/host_main.h"

#include <cstdlib>     // EXIT_SUCCESS / EXIT_FAILURE.

namespace rr = reap::rengine;
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
    host::host_state_t  host_state{};
    host_state.config.argc = argc;
    host_state.config.argv = argv;
    
    if ( host::Host_Init( host_state ) != host::host_error_code_t::OK ) {
        return ( EXIT_FAILURE );
    }
    
    while( host::Host_IsRunning( host_state ) ) {
        host::Host_BeginFrame( host_state );
        host::Host_Update( host_state );
        host::Host_Render( host_state );
        host::Host_EndFrame( host_state );
    }
    
    host::Host_Shutdown( host_state );
    
    return ( EXIT_SUCCESS );
}

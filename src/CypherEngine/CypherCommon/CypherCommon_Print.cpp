/*======================================================================
   File: com_print.cpp
   Project: CypherEngine
   Author: ksiric <email@example.com>
   Created: 2026-04-21 13:11:03
   Last Modified by: ksiric
   Last Modified: 2026-04-21 20:51:07
   ---------------------------------------------------------------------
   Description:

   ---------------------------------------------------------------------
   License:
   Company:
   Version: 0.1.0
 ======================================================================
                                                                       */

#include "CypherEngine/CypherCommon/CypherCommon_Print.h"

#include <cstdarg>     // va_list handling.
#include <cstdio>      // stdio output and formatting.

namespace cypher::engine::common
{

/*
================
CypherCommon_Printf
================
*/
void CypherCommon_Printf( const char *message, ... ) {
    va_list args;
    va_start( args, message );
    CypherCommon_VPrintf( message, args );
    va_end( args );
}

/*
================
CypherCommon_VPrintf

Formats normal engine text and writes it to stdout.
================
*/
void CypherCommon_VPrintf( const char *message, va_list args ) {
    char msg_buf[COM_MSG_MAX]{};
    const char *safe_message = message ? message : "<null message>";
    std::vsnprintf( msg_buf, sizeof( msg_buf ), safe_message, args );

    // Later this should route through registered print sinks.
    std::fputs( msg_buf, stdout );
    std::fflush( stdout );
}

/*
================
CypherCommon_DPrintf

Developer-print hook; later this should respect the developer cvar.
================
*/
void CypherCommon_DPrintf( const char *message, ... ) {
    // Developer-only printing will later be gated by the developer cvar.

    va_list args;
    va_start( args, message );
    CypherCommon_VPrintf( message, args );
    va_end( args );
}

/*
================
CypherCommon_Errorf
================
*/
void CypherCommon_Errorf( const error_t error, const char *message, ... ) {
    va_list args;
    va_start( args, message );
    CypherCommon_VErrorf( error, message, args );
    va_end( args );
}

/*
================
CypherCommon_VErrorf

Formats a domain-coded engine error and writes it to stderr.
================
*/
void CypherCommon_VErrorf( const error_t error, const char *message, va_list args ) {
    char msg_buf[COM_MSG_MAX]{};
    char msg_final[COM_MSG_MAX + 256]{};
    const char *safe_format = message ? message : "<null error message>";
    std::vsnprintf( msg_buf, sizeof( msg_buf ), safe_format, args );
    const domain_t domain = CypherCommon_ErrorDomain( error );

    std::snprintf(msg_final,
                  sizeof( msg_final ),
                  "[ERROR][%s][0x%08X] %s\n",
                  CypherCommon_DomainName( domain ),
                  static_cast<unsigned int>( error ),
                  msg_buf
                  );

    std::fputs( msg_final, stderr );
    std::fflush( stderr );
}

}       // namespace cypher::engine::common

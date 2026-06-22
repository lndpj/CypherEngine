#ifndef CYPHER_COMMON_TIER1_NAMESERVICEADDRESS_H
#define CYPHER_COMMON_TIER1_NAMESERVICEADDRESS_H
#pragma once

/*
================
CypherCommon Name Service Address

Named network endpoint declarations.
================
*/

#include "CypherCommon_NetAddress.h"

namespace cypher::common
{

struct name_service_address_t {
    char host[256];
    u16 port;
};

bool_t NameServiceAddress_Parse( const char *pString, name_service_address_t *pOutAddress );
bool_t NameServiceAddress_Resolve( const name_service_address_t *pAddress, net_address_t *pOutAddress );

} // namespace cypher::common

#endif // CYPHER_COMMON_TIER1_NAMESERVICEADDRESS_H

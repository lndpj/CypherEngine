#ifndef CYPHER_COMMON_TIER1_INTERFACE_H
#define CYPHER_COMMON_TIER1_INTERFACE_H
#pragma once

/*
================
CypherCommon Interface

Named interface registry declarations.
================
*/

#include "CypherCommon_Tier0.h"

namespace cypher::common
{

using interface_factory_t = void *( * )( const char *pName );

void Interface_RegisterFactory( interface_factory_t factory );
void *Interface_Create( const char *pName );

} // namespace cypher::common

#endif // CYPHER_COMMON_TIER1_INTERFACE_H

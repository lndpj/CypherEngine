#ifndef CYPHER_COMMON_TIER1_DATAMANAGER_H
#define CYPHER_COMMON_TIER1_DATAMANAGER_H
#pragma once

/*
================
CypherCommon Data Manager

Named data registry declarations.
================
*/

#include "CypherCommon_Tier0.h"

namespace cypher::common
{

struct data_manager_t;

bool_t DataManager_Register( data_manager_t *pManager, const char *pName, void *pData );
void *DataManager_Find( data_manager_t *pManager, const char *pName );
bool_t DataManager_Remove( data_manager_t *pManager, const char *pName );

} // namespace cypher::common

#endif // CYPHER_COMMON_TIER1_DATAMANAGER_H

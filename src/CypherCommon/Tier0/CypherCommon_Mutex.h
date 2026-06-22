#ifndef CYPHER_COMMON_TIER0_MUTEX_H
#define CYPHER_COMMON_TIER0_MUTEX_H
#pragma once

/*
================
CypherCommon Mutex

Low-level mutex declarations.
================
*/

#include "CypherCommon_BaseTypes.h"

namespace cypher::common
{

struct mutex_t;
struct recursive_mutex_t;

bool_t Mutex_Init( mutex_t *pMutex );
void Mutex_Shutdown( mutex_t *pMutex );
void Mutex_Lock( mutex_t *pMutex );
bool_t Mutex_TryLock( mutex_t *pMutex );
void Mutex_Unlock( mutex_t *pMutex );

} // namespace cypher::common

#endif // CYPHER_COMMON_TIER0_MUTEX_H

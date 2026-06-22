#ifndef CYPHER_COMMON_TIER1_COMMANDBUFFER_H
#define CYPHER_COMMON_TIER1_COMMANDBUFFER_H
#pragma once

/*
================
CypherCommon Command Buffer

Queued text command declarations.
================
*/

#include "CypherCommon_Tier0.h"

namespace cypher::common
{

struct command_buffer_t;

bool_t CommandBuffer_AddText( command_buffer_t *pBuffer, const char *pText );
bool_t CommandBuffer_GetNext( command_buffer_t *pBuffer, char *pDest, usize cchDest );
void CommandBuffer_Clear( command_buffer_t *pBuffer );

} // namespace cypher::common

#endif // CYPHER_COMMON_TIER1_COMMANDBUFFER_H

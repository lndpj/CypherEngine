#ifndef CYPHER_COMMON_TIER0_MEMORYDEBUG_H
#define CYPHER_COMMON_TIER0_MEMORYDEBUG_H
#pragma once

/*
================
CypherCommon Memory Debug

Memory debug event declarations.
================
*/

#include "CypherCommon_BaseTypes.h"

namespace cypher::common
{

enum class memory_debug_event_t : u32 {
    Alloc = 0u,
    Free,
    Realloc,
    Leak
};

using memory_debug_callback_t = void ( * )( memory_debug_event_t event_type,
                                            void *pMemory,
                                            usize cbSize,
                                            const char *pTag );

void MemoryDebug_SetCallback( memory_debug_callback_t callback );
void MemoryDebug_ReportEvent( memory_debug_event_t event_type, void *pMemory, usize cbSize, const char *pTag );

} // namespace cypher::common

#endif // CYPHER_COMMON_TIER0_MEMORYDEBUG_H

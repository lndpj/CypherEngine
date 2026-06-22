#ifndef CYPHER_COMMON_TIER1_EVENT_H
#define CYPHER_COMMON_TIER1_EVENT_H
#pragma once

/*
================
CypherCommon Event

Small event and callback declarations for tools, editor notifications and
runtime message routing.
================
*/

#include "CypherCommon_Tier0.h"

namespace cypher::common
{

using event_id_t = u32;

struct event_payload_t {
    const void *pData;
    usize cbData;
};

using event_callback_t = void ( * )( event_id_t eventId, const event_payload_t &payload, void *pUserData );

struct event_bus_t;

bool_t EventBus_Init( event_bus_t *pBus, u32 maxListeners );
void EventBus_Shutdown( event_bus_t *pBus );
bool_t EventBus_Subscribe( event_bus_t *pBus, event_id_t eventId, event_callback_t pCallback, void *pUserData );
bool_t EventBus_Unsubscribe( event_bus_t *pBus, event_id_t eventId, event_callback_t pCallback, void *pUserData );
bool_t EventBus_Emit( event_bus_t *pBus, event_id_t eventId, const event_payload_t &payload );

} // namespace cypher::common

#endif // CYPHER_COMMON_TIER1_EVENT_H

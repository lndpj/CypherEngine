#ifndef CYPHER_COMMON_TIER1_PACKETBUFFER_H
#define CYPHER_COMMON_TIER1_PACKETBUFFER_H
#pragma once

/*
================
CypherCommon Packet Buffer

Network packet byte buffer declarations.
================
*/

#include "CypherCommon_Tier0.h"

namespace cypher::common
{

struct packet_buffer_t {
    byte *pData;
    usize cbSize;
    usize cbCapacity;
};

void PacketBuffer_Init( packet_buffer_t *pBuffer, void *pData, usize cbCapacity );
bool_t PacketBuffer_Write( packet_buffer_t *pBuffer, const void *pData, usize cbData );
void PacketBuffer_Clear( packet_buffer_t *pBuffer );

} // namespace cypher::common

#endif // CYPHER_COMMON_TIER1_PACKETBUFFER_H

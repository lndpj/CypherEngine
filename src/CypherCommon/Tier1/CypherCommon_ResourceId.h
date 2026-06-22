#ifndef CYPHER_COMMON_TIER1_RESOURCEID_H
#define CYPHER_COMMON_TIER1_RESOURCEID_H
#pragma once

/*
================
CypherCommon Resource ID

Stable resource identifier declarations for assets, renderer objects, sounds,
materials, maps and editor references.
================
*/

#include "CypherCommon_Tier0.h"

namespace cypher::common
{

enum class resource_type_t : u16 {
    Unknown = 0u,
    Texture,
    Material,
    Mesh,
    Shader,
    Sound,
    Map,
    Script,
    Entity,
    Prefab
};

struct resource_id_t {
    hash64_t hash;
    resource_type_t type;
};

resource_id_t ResourceId_FromPath( resource_type_t type, const char *pVirtualPath );
resource_id_t ResourceId_FromName( resource_type_t type, const char *pName );
bool_t ResourceId_IsValid( resource_id_t id );
bool_t ResourceId_Equals( resource_id_t a, resource_id_t b );
void ResourceId_ToString( resource_id_t id, char *pDest, usize cchDest );

} // namespace cypher::common

#endif // CYPHER_COMMON_TIER1_RESOURCEID_H

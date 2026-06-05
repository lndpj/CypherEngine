#pragma once

#include "CypherEngine/CypherMath/CypherMath_Types.h"

namespace cypher::engine::math
{
 
frustum_t CypherMath_FrustumFromProjectionView( const mat4_t &projection_view );
    
bool CypherMath_FrustumContainsPoint( const frustum_t &frustum, const vec3_t &point );

bool CypherMath_FrustumIntersectsBounds( const frustum_t &frustum, const bounds_t &bounds );  
    
}       // namespace cypher::engine::math

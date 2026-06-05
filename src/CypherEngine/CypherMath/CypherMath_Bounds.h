#pragma once

#include "CypherEngine/CypherMath/CypherMath_Types.h"

namespace cypher::engine::math
{

bounds_t CypherMath_BoundsClear();

bounds_t CypherMath_BoundsFromPoint( const vec3_t &v );

void CypherMath_BoundsAddPoint( bounds_t &bounds, const vec3_t &point );

vec3_t CypherMath_BoundsCenter( const bounds_t &bounds );

vec3_t CypherMath_BoundsSize( const bounds_t &bounds );

bool CypherMath_BoundsContainsPoint( const bounds_t &bounds, const vec3_t &point );

bool CypherMath_BoundsIntersects( const bounds_t &b1, const bounds_t &b2 );
    
}       // namespace cypher::engine::math

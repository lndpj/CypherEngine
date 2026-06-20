#ifndef CYPHER_ENGINE_MATH_FRUSTUM_H
#define CYPHER_ENGINE_MATH_FRUSTUM_H

#pragma once

#include "CypherEngine/CypherMath/CypherMath_Types.h"

namespace cypher::engine::math
{

frustum_t CypherMath_FrustumFromProjectionView( const mat4_t &projectionView );

bool CypherMath_FrustumContainsPoint( const frustum_t &frustum, const vec3_t &point );

bool CypherMath_FrustumIntersectsBounds( const frustum_t &frustum, const bounds_t &bounds );

}       // namespace cypher::engine::math

#endif // CYPHER_ENGINE_MATH_FRUSTUM_H

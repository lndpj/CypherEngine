#ifndef CYPHER_ENGINE_MATH_RAY_H
#define CYPHER_ENGINE_MATH_RAY_H

#pragma once

#include "CypherEngine/CypherMath/CypherMath_Types.h"

namespace cypher::engine::math
{

vec3_t CypherMath_RayPointAt( const ray_t &ray, common::f32 t_units );

bool CypherMath_RayIntersectsPlane( const ray_t &ray, const plane_t &plane, common::f32 &out_t_units );

bool CypherMath_RayIntersectsBounds( const ray_t &ray, const bounds_t &bounds, common::f32 &out_tmin, common::f32 &out_tmax );
    
}       // namespace cypher::engine::math

#endif // CYPHER_ENGINE_MATH_RAY_H

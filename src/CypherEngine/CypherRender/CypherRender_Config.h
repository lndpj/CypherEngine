#ifndef CYPHER_ENGINE_RENDER_CONFIG_H
#define CYPHER_ENGINE_RENDER_CONFIG_H

#pragma once

#include "CypherEngine/CypherMath/CypherMath_Types.h"
#include "CypherEngine/CypherCommon/CypherCommon.h"

namespace cypher::engine::render
{

// DEFAULT CONSTANTS FOR THE RENDERER, LATER IN THE CONFIG DEFINED
constexpr common::f32 CYPHER_RENDER_DEFAULT_FOV_Y_RADIANS      = 70.0f * math::MATH_DEG2RAD_F;
constexpr common::f32 CYPHER_RENDER_DEFAULT_NEAR_Z             = 0.1f;
constexpr common::f32 CYPHER_RENDER_DEFAULT_FAR_Z              = 4096.0f;

constexpr common::f32 CYPHER_RENDER_DEFAULT_VIEWPORT_WIDTH     = 1280u;
constexpr common::f32 CYPHER_RENDER_DEFAULT_VIEWPORT_HEIGHT    = 720u;
constexpr common::f32 CYPHER_RENDER_DEFAULT_ASPECT_RATIO       = ( CYPHER_RENDER_DEFAULT_VIEWPORT_WIDTH / CYPHER_RENDER_DEFAULT_VIEWPORT_HEIGHT );
    
}       // namespace cypher::engine::render

#endif // CYPHER_ENGINE_RENDER_CONFIG_H

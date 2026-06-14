#ifndef CYPHER_ENGINE_RENDER_DRAW_H
#define CYPHER_ENGINE_RENDER_DRAW_H

#pragma once

#include "CypherEngine/CypherMath/CypherMath_Types.h"
#include "CypherEngine/CypherRender/CypherRender_Camera.h"
#include "CypherEngine/CypherRender/CypherRender_Mesh.h"
#include "CypherEngine/CypherRender/CypherRender_Shader.h"

namespace cypher::engine::render
{
    
constexpr common::u32 CYPHER_RENDER_DRAW_ITEMS_LIST_MAX = 16384u;

struct draw_item_t {
    math::mat4_t model_matrix{};

    mesh_t    *mesh{};
    shader_t  *shader{};
};

struct draw_list_t {
    draw_item_t *items{ nullptr };
    common::u32 item_count{ 0u };
    common::u32 item_capacity{ 0u };
};

render_error_t CypherRender_DrawItem( const draw_item_t &item, const camera_t &camera );

void CypherRender_DrawListInit( draw_list_t &draw_list, draw_item_t *items, common::u32 item_capacity );

void CypherRender_DrawListClear( draw_list_t &draw_list );

render_error_t CypherRender_DrawListSubmit( draw_list_t &draw_list, const draw_item_t &item );

render_error_t CypherRender_DrawListDraw( const draw_list_t &draw_list, const camera_t &camera );

}       // namespace cypher::engine::render

#endif // CYPHER_ENGINE_RENDER_DRAW_H

#ifndef CYPHER_ENGINE_RENDER_DRAW_H
#define CYPHER_ENGINE_RENDER_DRAW_H

#pragma once

#include "CypherMath_Types.h"
#include "CypherRender_Camera.h"
#include "CypherRender_Mesh.h"
#include "CypherRender_Shader.h"

namespace cypher::engine::render
{

constexpr common::u32 CYPHER_RENDER_DRAW_ITEMS_LIST_MAX = 16384u;

struct draw_item_t {
    math::mat4_t modelMatrix{};

    mesh_t    *mesh{};
    shader_t  *shader{};
};

struct draw_list_t {
    draw_item_t *items{ nullptr };
    common::u32 nItemCount{ 0u };
    common::u32 nItemCapacity{ 0u };
};

render_error_t CypherRender_DrawItem( const draw_item_t &item, const camera_t &camera );

void CypherRender_DrawListInit( draw_list_t &drawList, draw_item_t *items, common::u32 nItemCapacity );

void CypherRender_DrawListClear( draw_list_t &drawList );

render_error_t CypherRender_DrawListSubmit( draw_list_t &drawList, const draw_item_t &item );

render_error_t CypherRender_DrawListDraw( const draw_list_t &drawList, const camera_t &camera );

}       // namespace cypher::engine::render

#endif // CYPHER_ENGINE_RENDER_DRAW_H

#pragma once

#include "CypherEngine/CypherMath/CypherMath_Types.h"
#include "CypherEngine/CypherRender/CypherRender_Camera.h"
#include "CypherEngine/CypherRender/CypherRender_Mesh.h"
#include "CypherEngine/CypherRender/CypherRender_Shader.h"

namespace cypher::engine::render
{
    
constexpr common::u32 CYPHER_RENDER_DRAW_ITEMS_LIST_MAX = 16384u;

struct cypher_render_draw_item_t {
    math::mat4_t model_matrix{};

    cypher_render_mesh_t    *mesh{};
    cypher_render_shader_t  *shader{};
};

struct cypher_render_draw_list_t {
    cypher_render_draw_item_t *items{ nullptr };
    common::u32 item_count{ 0u };
    common::u32 item_capacity{ 0u };
};

cypher_render_error_code_t CypherRender_DrawItem( const cypher_render_draw_item_t &item, const cypher_render_camera_t &camera );

void CypherRender_DrawListInit( cypher_render_draw_list_t &draw_list, cypher_render_draw_item_t *items, common::u32 item_capacity );

void CypherRender_DrawListClear( cypher_render_draw_list_t &draw_list );

cypher_render_error_code_t CypherRender_DrawListSubmit( cypher_render_draw_list_t &draw_list, const cypher_render_draw_item_t &item );

cypher_render_error_code_t CypherRender_DrawListDraw( const cypher_render_draw_list_t &draw_list, const cypher_render_camera_t &camera );

}       // namespace cypher::engine::render

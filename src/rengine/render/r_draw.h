#pragma once

#include "rengine/math/math_types.h"
#include "rengine/render/r_camera.h"
#include "rengine/render/r_mesh.h"
#include "rengine/render/r_shader.h"

namespace reap::rengine::render
{
    
constexpr rcommon::u32 R_DRAW_ITEMS_LIST_MAX = 16384u;

struct r_draw_item_t {
    math::mat4_t model_matrix{};

    r_mesh_t    *mesh{};
    r_shader_t  *shader{};
};

struct r_draw_list_t {
    r_draw_item_t items[R_DRAW_ITEMS_LIST_MAX]{};
    rcommon::u32 item_count{ 0u };
};

r_error_code_t R_DrawItem( const r_draw_item_t &item, const r_camera_t &camera );

void R_DrawListClear( r_draw_list_t &draw_list );

r_error_code_t R_DrawListSubmit( r_draw_list_t &draw_list, const r_draw_item_t &item );

r_error_code_t R_DrawListDraw( const r_draw_list_t &draw_list, const r_camera_t &camera );

}       // namespace reap::rengine::render

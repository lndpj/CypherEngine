#pragma once

#include "rengine/math/math_types.h"
#include "rengine/render/r_camera.h"
#include "rengine/render/r_mesh.h"
#include "rengine/render/r_shader.h"

namespace reap::rengine::render
{

struct r_draw_item_t {
    math::vec3_t position{};  
    math::quat_t orientation{};  
    math::vec3_t scale{};  
    
    r_mesh_t    *mesh{};
    r_shader_t  *shader{};
};

struct r_draw_list_t {
      
};

r_error_code_t R_DrawObject( const r_draw_item_t &item, const r_camera_t &camera );

}       // namespace reap::rengine::render

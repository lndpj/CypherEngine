#pragma once

#include "rengine/host/host_types.h"
#include "rengine/render/r_camera.h"
#include "rengine/render/r_draw.h"
#include "rengine/render/r_error.h"
#include "rengine/render/r_gl.h"
#include "rengine/render/r_mesh.h"
#include "rengine/render/r_shader.h"

namespace reap::rengine::render
{

/*
================
Renderer Runtime State

High-level renderer state. Backend-specific OpenGL details stay behind r_gl.
================
*/
struct render_runtime_state_t {
    bool initialized{ false };
    bool in_frame{ false };
    
    const sys::sys_window_t *window{ nullptr };
    
    r_camera_t active_camera{};
    
    r_draw_list_t main_draw_list{};
    
    rcommon::u32 viewport_width{ 0u };    
    rcommon::u32 viewport_height{ 0u };    
    
    r_gl_state_t gl_state{};
    r_shader_registry_t shader_registry{};
    r_shader_t *basic_shader{ nullptr };
    r_mesh_t test_mesh{};
};

/*
================
Renderer API
================
*/
r_error_code_t R_Init( const sys::sys_window_t &window, const host::window_config_t &window_config );

void R_Shutdown();

r_error_code_t R_BeginFrame( const rcommon::f32 delta_time_seconds );

r_error_code_t R_RenderFrame();

r_error_code_t R_EndFrame();

r_error_code_t R_SubmitDrawItem( const r_draw_item_t &draw_item );

bool R_IsInitialized();

}

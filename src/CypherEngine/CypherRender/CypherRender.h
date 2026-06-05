#pragma once

#include "CypherEngine/CypherHost/CypherHost_Types.h"
#include "CypherEngine/CypherRender/CypherRender_Camera.h"
#include "CypherEngine/CypherRender/CypherRender_Draw.h"
#include "CypherEngine/CypherRender/CypherRender_Error.h"
#include "CypherEngine/CypherRender/CypherRender_GL.h"
#include "CypherEngine/CypherRender/CypherRender_Mesh.h"
#include "CypherEngine/CypherRender/CypherRender_Shader.h"

namespace cypher::engine::render
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
    
    const sys::cypher_system_window_t *window{ nullptr };
    
    cypher_render_camera_t active_camera{};
    
    cypher_render_draw_list_t main_draw_list{};
    
    common::u32 viewport_width{ 0u };    
    common::u32 viewport_height{ 0u };    
    
    cypher_render_gl_state_t gl_state{};
    cypher_render_shader_registry_t shader_registry{};
    cypher_render_shader_t *basic_shader{ nullptr };
    cypher_render_mesh_t test_mesh{};
};

/*
================
Renderer API
================
*/
cypher_render_error_code_t CypherRender_Init( const sys::cypher_system_window_t &window, const host::window_config_t &window_config );

void CypherRender_Shutdown();

cypher_render_error_code_t CypherRender_BeginFrame( const common::f32 delta_time_seconds );

cypher_render_error_code_t CypherRender_RenderFrame();

cypher_render_error_code_t CypherRender_EndFrame();

cypher_render_error_code_t CypherRender_SubmitDrawItem( const cypher_render_draw_item_t &draw_item );

bool CypherRender_IsInitialized();

}

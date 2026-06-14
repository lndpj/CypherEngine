#ifndef CYPHER_ENGINE_RENDER_H
#define CYPHER_ENGINE_RENDER_H

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
    
    const sys::window_t *window{ nullptr };
    
    camera_t active_camera{};
    
    draw_list_t main_draw_list{};
    
    common::u32 viewport_width{ 0u };    
    common::u32 viewport_height{ 0u };    
    
    gl_state_t gl_state{};
    shader_registry_t shader_registry{};
};

/*
================
Renderer API
================
*/
render_error_t CypherRender_Init( const sys::window_t &window, const host::window_config_t &window_config );

void CypherRender_Shutdown();

render_error_t CypherRender_BeginFrame( const common::f32 delta_time_seconds );

render_error_t CypherRender_RenderFrame();

render_error_t CypherRender_EndFrame();

render_error_t CypherRender_SubmitDrawItem( const draw_item_t &draw_item );

bool CypherRender_IsInitialized();

}

#endif // CYPHER_ENGINE_RENDER_H

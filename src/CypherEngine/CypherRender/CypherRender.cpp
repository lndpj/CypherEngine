/*======================================================================
   File: CypherRender.cpp
   Project: CypherEngine
   Author: ksiric <email@example.com>
   Created: 2026-04-20 21:01:21
   Last Modified by: ksiric
   Last Modified: 2026-06-05 11:55:38
   ---------------------------------------------------------------------
   Description:

   ---------------------------------------------------------------------
   License:
   Company:
   Version: 0.1.0
 ======================================================================
																	   */
#include "CypherEngine/CypherRender/CypherRender.h"
#include "CypherEngine/CypherLog/CypherLog.h"
#include "CypherEngine/CypherRender/CypherRender_Camera.h"
#include "CypherEngine/CypherRender/CypherRender_Draw.h"
#include "CypherEngine/CypherRender/CypherRender_GL.h"

namespace cypher::engine::render {

// @NOTE will be replaced by arena allocators soon
namespace
{
draw_item_t g_main_draw_items[CYPHER_RENDER_DRAW_ITEMS_LIST_MAX]{};
}

render_runtime_state_t g_render_runtime_state{};

/*
================
CypherRender_Init

Validates host/window config, starts the GL backend and initializes renderer state.
================
*/
error_code_t CypherRender_Init( const sys::window_t &window, const host::window_config_t &window_config ) {
	if ( CypherRender_IsInitialized() ) {
		CYPHER_LOG_WARNING( log::channel_t::RENDER, "renderer already initialized." );
		return error_code_t::ERR_IS_INIT;
	}

	if ( !window.valid || window.native_window == nullptr ) {
		CYPHER_LOG_ERROR( log::channel_t::RENDER, "invalid sys window." );
		return error_code_t::ERR_INVALID_WINDOW_CFG;
	}

	if ( window_config.viewport.width == 0u || window_config.viewport.height == 0u ) {
		CYPHER_LOG_ERROR(
			log::channel_t::RENDER,
			"invalid viewport %ux%u (both dimensions must be > 0).",
			window_config.viewport.width,
			window_config.viewport.height );
		return error_code_t::ERR_INVALID_VIEWPORT;
	}

	g_render_runtime_state.window = &window;
	g_render_runtime_state.viewport_width = window_config.viewport.width;
	g_render_runtime_state.viewport_height = window_config.viewport.height;
	g_render_runtime_state.gl_state = {};
	g_render_runtime_state.shader_registry = {};
	g_render_runtime_state.in_frame = false;

    CypherRender_DrawListInit(
        g_render_runtime_state.main_draw_list,
        g_main_draw_items,
        CYPHER_RENDER_DRAW_ITEMS_LIST_MAX );

	CYPHER_LOG_INFO(
		log::channel_t::RENDER,
		"renderer initialized with viewport %ux%u.",
		g_render_runtime_state.viewport_width,
		g_render_runtime_state.viewport_height );

	const auto gl_result = CypherRenderGL_Init( window, window_config.vsync, g_render_runtime_state.gl_state );
	if ( gl_result != error_code_t::OK ) {
        CYPHER_LOG_ERROR( log::channel_t::RENDER, "renderer backend initialization failed: %s.", CypherRender_ErrorDesc( gl_result ) );
		g_render_runtime_state = {};
		return gl_result;
	}

	CypherRender_ShaderRegistryInit( g_render_runtime_state.shader_registry );

    camera_desc_t camera_desc{};
    camera_desc.camera_projection_mode = camera_projection_mode_t::PERSPECTIVE;
    camera_desc.aspect_ratio    = static_cast<common::f32>( window_config.viewport.width ) / 
                                  static_cast<common::f32>( window_config.viewport.height );
    camera_desc.fov_y_radians   = CYPHER_RENDER_DEFAULT_FOV_Y_RADIANS;
    camera_desc.far_z           = CYPHER_RENDER_DEFAULT_FAR_Z;
    camera_desc.near_z          = CYPHER_RENDER_DEFAULT_NEAR_Z;
    CypherRender_CameraInit( g_render_runtime_state.active_camera, camera_desc );
    
    g_render_runtime_state.initialized = true;

	return error_code_t::OK;
}

/*
================
CypherRender_Shutdown
================
*/
void CypherRender_Shutdown() {
	if ( !CypherRender_IsInitialized() ) {
		CYPHER_LOG_INFO( log::channel_t::RENDER, "renderer was not initialized; nothing to shutdown." );
		return;
	}

	CypherRender_ShaderRegistryShutdown( g_render_runtime_state.shader_registry );
	CypherRenderGL_Shutdown( g_render_runtime_state.gl_state );

	g_render_runtime_state = {};

	CYPHER_LOG_INFO( log::channel_t::RENDER, "renderer shutdown complete." );
}

/*
================
CypherRender_BeginFrame

Opens a render frame and prepares the backend for drawing.
================
*/
error_code_t CypherRender_BeginFrame( const common::com_f32 delta_time_seconds ) {
	if ( !CypherRender_IsInitialized() ) {
        CYPHER_LOG_ERROR( log::channel_t::RENDER, "begin frame failed: renderer is not initialized." );
		return error_code_t::ERR_NOT_INIT;
	}

	if ( g_render_runtime_state.in_frame ) {
        CYPHER_LOG_ERROR( log::channel_t::RENDER, "begin frame failed: frame is already active." );
		return error_code_t::ERR_FRAME_ALREADY_ACTIVE;
	}

	const auto gl_result = CypherRenderGL_BeginFrame( *g_render_runtime_state.window );
	if ( gl_result != error_code_t::OK ) {
        CYPHER_LOG_ERROR( log::channel_t::RENDER, "begin frame failed: GL begin failed: %s.", CypherRender_ErrorDesc( gl_result ) );
		return error_code_t::ERR_BEGIN_DRAW;
	}

    CypherRender_DrawListClear( g_render_runtime_state.main_draw_list );

	(void)delta_time_seconds;
	g_render_runtime_state.in_frame = true;

	return error_code_t::OK;
}

/*
================
CypherRender_RenderFrame

Draws the items submitted by world/game/editor systems for the active frame.
================
*/
error_code_t CypherRender_RenderFrame() {
	if ( !CypherRender_IsInitialized() ) {
        CYPHER_LOG_ERROR( log::channel_t::RENDER, "render frame failed: renderer is not initialized." );
		return error_code_t::ERR_NOT_INIT;
	}

	if ( !g_render_runtime_state.in_frame ) {
        CYPHER_LOG_ERROR( log::channel_t::RENDER, "render frame failed: frame is not active." );
		return error_code_t::ERR_FRAME_NOT_ACTIVE;
	}

    return CypherRender_DrawListDraw(
        g_render_runtime_state.main_draw_list,
        g_render_runtime_state.active_camera );
}

/*
================
CypherRender_EndFrame

Closes the render frame and presents the back buffer.
================
*/
error_code_t CypherRender_EndFrame() {
	if ( !CypherRender_IsInitialized() ) {
        CYPHER_LOG_ERROR( log::channel_t::RENDER, "end frame failed: renderer is not initialized." );
		return error_code_t::ERR_NOT_INIT;
	}

	if ( !g_render_runtime_state.in_frame ) {
        CYPHER_LOG_ERROR( log::channel_t::RENDER, "end frame failed: frame is not active." );
		return error_code_t::ERR_FRAME_NOT_ACTIVE;
	}

	const auto gl_result = CypherRenderGL_EndFrame( *g_render_runtime_state.window );
	if ( gl_result != error_code_t::OK ) {
        CYPHER_LOG_ERROR( log::channel_t::RENDER, "end frame failed: GL end failed: %s.", CypherRender_ErrorDesc( gl_result ) );
		return error_code_t::ERR_END_DRAW;
	}

	g_render_runtime_state.in_frame = false;

	return error_code_t::OK;
}

/*
================
CypherRender_SubmitDrawItem

Submits one draw item for the current frame. Game, editor and ECS layers use
this as the public doorway into the renderer draw list.
================
*/
error_code_t CypherRender_SubmitDrawItem( const draw_item_t &draw_item )
{
    if ( !CypherRender_IsInitialized() ) {
        CYPHER_LOG_ERROR( log::channel_t::RENDER, "submit draw item failed: renderer is not initialized." );
        return error_code_t::ERR_NOT_INIT;
    }
    
    if ( !g_render_runtime_state.in_frame ) {
        CYPHER_LOG_ERROR( log::channel_t::RENDER, "submit draw item failed: frame is not active." );
        return error_code_t::ERR_FRAME_NOT_ACTIVE;
    }
    
    return CypherRender_DrawListSubmit( g_render_runtime_state.main_draw_list, draw_item );
}

/*
================
CypherRender_IsInitialized
================
*/
bool CypherRender_IsInitialized() {
	return g_render_runtime_state.initialized;
}

} // namespace cypher::engine::render

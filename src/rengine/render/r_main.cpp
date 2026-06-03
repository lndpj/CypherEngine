/*======================================================================
   File: r_main.cpp
   Project: REAP
   Author: ksiric <email@example.com>
   Created: 2026-04-20 21:01:21
   Last Modified by: ksiric
   Last Modified: 2026-06-03 18:18:32
   ---------------------------------------------------------------------
   Description:

   ---------------------------------------------------------------------
   License:
   Company:
   Version: 0.1.0
 ======================================================================
																	   */
#include "rengine/render/r_main.h"
#include "rengine/log/log_main.h"
#include "rengine/render/r_camera.h"
#include "rengine/render/r_gl.h"

#include <SDL3/SDL.h>      // Temporary renderer-side SDL visibility while backend matures.

namespace reap::rengine::render {

render_runtime_state_t g_render_runtime_state{};

/*
================
R_Init

Validates host/window config, starts the GL backend and loads core shaders.
================
*/
r_error_code_t R_Init( const sys::sys_window_t &window, const host::window_config_t &window_config ) {
	if ( R_IsInitialized() ) {
		REAP_LOG_WARNING( log::log_channel_t::RENDER, "renderer already initialized." );
		return r_error_code_t::ERR_IS_INIT;
	}

	if ( !window.valid || window.native_window == nullptr ) {
		REAP_LOG_ERROR( log::log_channel_t::RENDER, "invalid sys window." );
		return r_error_code_t::ERR_INVALID_WINDOW_CFG;
	}

	if ( window_config.viewport.width == 0u || window_config.viewport.height == 0u ) {
		REAP_LOG_ERROR(
			log::log_channel_t::RENDER,
			"invalid viewport %ux%u (both dimensions must be > 0).",
			window_config.viewport.width,
			window_config.viewport.height );
		return r_error_code_t::ERR_INVALID_VIEWPORT;
	}

	g_render_runtime_state.window = &window;
	g_render_runtime_state.viewport_width = window_config.viewport.width;
	g_render_runtime_state.viewport_height = window_config.viewport.height;
	g_render_runtime_state.gl_state = {};
	g_render_runtime_state.shader_registry = {};
	g_render_runtime_state.in_frame = false;

	REAP_LOG_INFO(
		log::log_channel_t::RENDER,
		"renderer initialized with viewport %ux%u.",
		g_render_runtime_state.viewport_width,
		g_render_runtime_state.viewport_height );

	const auto gl_result = R_GLInit( window, window_config.vsync, g_render_runtime_state.gl_state );
	if ( gl_result != r_error_code_t::OK ) {
		g_render_runtime_state = {};
		return gl_result;
	}

	R_ShaderRegistryInit( g_render_runtime_state.shader_registry );

	r_shader_t *basic_shader{ nullptr };

	const auto shader_result = R_ShaderLoad( g_render_runtime_state.shader_registry, "basic_color", "data/shaders/gl/basic_color.vert",
											 "data/shaders/gl/basic_color.frag",
											 &basic_shader );
    if ( shader_result != r_error_code_t::OK ) {
        R_ShaderRegistryShutdown( g_render_runtime_state.shader_registry );
        R_GLShutdown( g_render_runtime_state.gl_state );
        g_render_runtime_state = {};
        return shader_result;
    }

    g_render_runtime_state.basic_shader = basic_shader;
    
    /*
    Temporary pipeline test mesh. Kept here as reference while real 3D
    transform/camera rendering is built.

    const r_vertex_t vertices[] = {
        { { -0.5f, -0.5f, 0.0f }, { 1.0f, 0.0f, 0.0f } },
        { {  0.5f, -0.5f, 0.0f }, { 0.0f, 1.0f, 0.0f } },
        { {  0.0f,  0.5f, 0.0f }, { 0.0f, 0.0f, 1.0f } },
    };

    const rcommon::u32 indices[] = {
        0u, 1u, 2u
    };

    const auto mesh_result = R_MeshCreate(
        vertices,
        3u,
        indices,
        3u,
        g_render_runtime_state.test_mesh );

    if ( mesh_result != r_error_code_t::OK ) {
        R_ShaderRegistryShutdown( g_render_runtime_state.shader_registry );
        R_GLShutdown( g_render_runtime_state.gl_state );
        g_render_runtime_state = {};
        return mesh_result;
    }
    */
    
    /*
     * Camera initialization
     */
    
    r_camera_desc_t camera_desc{};
    camera_desc.camera_projection_mode = r_camera_projection_mode_t::PERSPECTIVE;
    camera_desc.aspect_ratio    = static_cast<rcommon::f32>( window_config.viewport.width ) / 
                                  static_cast<rcommon::f32>( window_config.viewport.height );
    camera_desc.fov_y_radians   = R_DEFAULT_FOV_Y_RADIANS;
    camera_desc.far_z           = R_DEFAULT_FAR_Z;
    camera_desc.near_z          = R_DEFAULT_NEAR_Z;
    R_CameraInit( g_render_runtime_state.active_camera, camera_desc );
    
    g_render_runtime_state.initialized = true;

	return r_error_code_t::OK;
}

/*
================
R_Shutdown
================
*/
void R_Shutdown() {
	if ( !R_IsInitialized() ) {
		REAP_LOG_INFO( log::log_channel_t::RENDER, "renderer was not initialized; nothing to shutdown." );
		return;
	}

	R_MeshDestroy( g_render_runtime_state.test_mesh );
	R_ShaderRegistryShutdown( g_render_runtime_state.shader_registry );
	R_GLShutdown( g_render_runtime_state.gl_state );

	g_render_runtime_state = {};

	REAP_LOG_INFO( log::log_channel_t::RENDER, "renderer shutdown complete." );
}

/*
================
R_BeginFrame

Opens a render frame and prepares the backend for drawing.
================
*/
r_error_code_t R_BeginFrame( const rcommon::com_f32 delta_time_seconds ) {
	if ( !R_IsInitialized() ) {
		return r_error_code_t::ERR_NOT_INIT;
	}

	if ( g_render_runtime_state.in_frame ) {
		return r_error_code_t::ERR_FRAME_ALREADY_ACTIVE;
	}

	const auto gl_result = R_GLBeginFrame( *g_render_runtime_state.window );
	if ( gl_result != r_error_code_t::OK ) {
		return r_error_code_t::ERR_BEGIN_DRAW;
	}

	(void)delta_time_seconds;
	g_render_runtime_state.in_frame = true;

	return r_error_code_t::OK;
}

/*
================
R_RenderFrame

Submits scene rendering work for the active frame.
================
*/
r_error_code_t R_RenderFrame() {
	if ( !R_IsInitialized() ) {
		return r_error_code_t::ERR_NOT_INIT;
	}

	if ( !g_render_runtime_state.in_frame ) {
		return r_error_code_t::ERR_FRAME_NOT_ACTIVE;
	}

    /*
    Temporary pipeline test draw. Disabled now that the RGB triangle path is
    proven and we are moving toward real 3D transforms.

    if ( g_render_runtime_state.basic_shader == nullptr ) {
        return r_error_code_t::ERR_SHADER_BIND;
    }

    const auto shader_result = R_ShaderBind( *g_render_runtime_state.basic_shader );
    if ( shader_result != r_error_code_t::OK ) {
        return shader_result;
    }
	// return R_MeshDraw( g_render_runtime_state.test_mesh );
    */

    return r_error_code_t::OK;
}

/*
================
R_EndFrame

Closes the render frame and presents the back buffer.
================
*/
r_error_code_t R_EndFrame() {
	if ( !R_IsInitialized() ) {
		return r_error_code_t::ERR_NOT_INIT;
	}

	if ( !g_render_runtime_state.in_frame ) {
		return r_error_code_t::ERR_FRAME_NOT_ACTIVE;
	}

	const auto gl_result = R_GLEndFrame( *g_render_runtime_state.window );
	if ( gl_result != r_error_code_t::OK ) {
		return r_error_code_t::ERR_END_DRAW;
	}

	g_render_runtime_state.in_frame = false;

	return r_error_code_t::OK;
}

/*
================
R_IsInitialized
================
*/
bool R_IsInitialized() {
	return g_render_runtime_state.initialized;
}

} // namespace reap::rengine::render

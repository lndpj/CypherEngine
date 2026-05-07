#pragma once

#include "rengine/render/r_error.h"
#include "rengine/sys/sys_window.h"
namespace reap::rengine::render
{

struct r_gl_state_t {
    void *context{ nullptr };
};

r_error_code_t R_GLInit( const sys::sys_window_t &window, bool vsync, r_gl_state_t &gl_state );

void R_GLShutdown( r_gl_state_t &gl_state );

r_error_code_t R_GLBeginFrame( const sys::sys_window_t &window );

r_error_code_t R_GLEndFrame( const sys::sys_window_t &window );

r_error_code_t R_GLCreateShaderProgram( const char *vertex_source, const char *fragment_source, rcommon::u32 &out_shader_program_id );
    
}       // namespace reap::rengine::render

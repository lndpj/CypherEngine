#pragma once

#include "rengine/rcommon/com_main.h"
#include "rengine/sys/sys_error.h"

namespace reap::rengine::sys 
{

/*
================
System Window Types

The public engine-facing wrapper around native SDL window state.
================
*/
struct sys_window_desc_t {
    const char *title{ rcommon::COM_GAME_INFO.name };
    rcommon::u32 width{ 1280u };
    rcommon::u32 height{ 720u };
    bool fullscreen{ false };
    bool vsync{ true };
};

struct sys_window_t {
    void *native_window{ nullptr };
    
    rcommon::u32 width{ 0u };
    rcommon::u32 height{ 0u };
    
    bool fullscreen{ false};
    bool vsync{ true };
    bool should_close{ false };
    bool valid{ false };
};

/*
================
System Window API
================
*/
sys_error_code_t Sys_CreateWindow( const sys_window_desc_t &window_description, sys_window_t &out_window );

void Sys_DestroyWindow( sys_window_t &window );

void Sys_PollWindowEvents( sys_window_t &window );

bool Sys_WindowShouldClose( const sys_window_t &window ); 
    
}       // namespace reap::rengine::sys

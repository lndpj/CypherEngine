#pragma once

#include "CypherEngine/CypherCommon/CypherCommon.h"
#include "CypherEngine/CypherSystem/CypherSystem_Error.h"

namespace cypher::engine::sys 
{

/*
================
System Window Types

The public engine-facing wrapper around native SDL window state.
================
*/
struct window_desc_t {
    const char *title{ common::COM_GAME_INFO.name };
    common::u32 width{ 1280u };
    common::u32 height{ 720u };
    bool fullscreen{ false };
    bool vsync{ true };
};

struct window_t {
    void *native_window{ nullptr };
    
    common::u32 width{ 0u };
    common::u32 height{ 0u };
    
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
error_code_t CypherSystem_CreateWindow( const window_desc_t &window_description, window_t &out_window );

void CypherSystem_DestroyWindow( window_t &window );

void CypherSystem_PollWindowEvents( window_t &window );

bool CypherSystem_WindowShouldClose( const window_t &window ); 
    
}       // namespace cypher::engine::sys

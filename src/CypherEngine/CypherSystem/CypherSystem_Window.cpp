/*======================================================================
   File: sys_window.cpp
   Project: CypherEngine
   Author: ksiric <email@example.com>
   Created: 2026-05-04 02:29:40
   Last Modified by: ksiric
   Last Modified: 2026-05-05 21:55:33
   ---------------------------------------------------------------------
   Description:
       
   ---------------------------------------------------------------------
   License: 
   Company: 
   Version: 0.1.0
 ======================================================================
                                                                       */
#include "CypherEngine/CypherSystem/CypherSystem_Window.h"
#include "CypherEngine/CypherCommon/CypherCommon_Print.h"

#include <SDL3/SDL.h>      // Cross-platform window and event API.

namespace cypher::engine::sys
{

/*
================
CypherSystem_CreateWindow

Creates the SDL window used later by the renderer backend.
================
*/
error_code_t CypherSystem_CreateWindow( const window_desc_t &window_description, window_t &out_window )
{
    SDL_WindowFlags flags{};
    SDL_Window *sdl_window{ nullptr };
    
    if ( window_description.title == nullptr || window_description.title[0] == '\0' ) {
        return error_code_t::ERR_INVALID_ARGUMENT;
    } 
    
    if ( window_description.width == 0u || window_description.height == 0u ) {
        return error_code_t::ERR_INVALID_ARGUMENT;
    }
    
    if ( out_window.native_window != nullptr ) {
        return error_code_t::ERR_IS_INIT;
    }
    
    if ( !SDL_InitSubSystem( SDL_INIT_VIDEO ) ) {
        common::CypherCommon_Printf( "CypherSystem_CreateWindow: SDL_InitSubSystem failed: %s\n", SDL_GetError() );
        return error_code_t::ERR_INTERNAL_ERROR;
    }
    
    flags = SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL | SDL_WINDOW_HIGH_PIXEL_DENSITY;
    
    if ( window_description.fullscreen ) {
        flags |= SDL_WINDOW_FULLSCREEN;
    }
    
    sdl_window = SDL_CreateWindow( window_description.title, static_cast<int>( window_description.width ), static_cast<int>( window_description.height ), flags );
    
    if ( sdl_window == nullptr ) {
        common::CypherCommon_Printf( "CypherSystem_CreateWindow: SDL_CreateWindow failed: %s\n", SDL_GetError() );
        SDL_QuitSubSystem( SDL_INIT_VIDEO );
        return error_code_t::ERR_INTERNAL_ERROR;
    } 
    
    out_window.native_window = sdl_window;
    out_window.fullscreen = window_description.fullscreen;
    out_window.width = window_description.width;
    out_window.height = window_description.height;
    out_window.vsync = window_description.vsync;
    out_window.should_close = false;
    out_window.valid = true;
    
    return error_code_t::OK;   
}

/*
================
CypherSystem_DestroyWindow
================
*/
void CypherSystem_DestroyWindow( window_t &window ) {
    SDL_Window *sdl_window{ nullptr };
    
    if ( window.native_window == nullptr ) {
        return ;
    }  
    
    sdl_window = static_cast<SDL_Window *>( window.native_window );
    
    SDL_DestroyWindow( sdl_window );
    SDL_QuitSubSystem( SDL_INIT_VIDEO ); 
    
    window = {};
   
    return ; 
}

/*
================
CypherSystem_PollWindowEvents

Updates window state from SDL events.
================
*/
void CypherSystem_PollWindowEvents( window_t &window ) {
    SDL_Event event{};
    
    if ( window.native_window == nullptr ) {
        return ;
    }
    
    while ( SDL_PollEvent( &event ) ) {
        switch( event.type ) {
            case SDL_EVENT_QUIT:
                window.should_close = true;
                break;
            case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
                window.should_close = true;
                break;
            case SDL_EVENT_WINDOW_RESIZED:
                if ( event.window.data1 > 0 && event.window.data2 > 0 ) {
                    window.width = static_cast<common::u32>( event.window.data1 );
                    window.height = static_cast<common::u32>( event.window.data2 );
                }
                break;
            default:
                break;
        }
    }
}

/*
================
CypherSystem_WindowShouldClose
================
*/
bool CypherSystem_WindowShouldClose( const window_t &window ) {
    return window.should_close;   
}
    
}       // namespace cypher::engine::sys

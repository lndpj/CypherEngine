#pragma once

#include "rengine/sys/sys_platform.h"

#include <glad/gl.h>       // OpenGL function declarations loaded by GLAD.
#include <SDL3/SDL.h>      // SDL_GL context attribute constants.

/*
================
OpenGL Platform Defaults

macOS is capped at OpenGL 4.1 core, while Windows/Linux can request newer GL.
================
*/
#if REAP_PLATFORM_MACOS
    constexpr int SYS_GL_CONTEXT_MAJOR = 4;   
    constexpr int SYS_GL_CONTEXT_MINOR = 1;
    constexpr int SYS_GL_CONTEXT_FLAGS = SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG;
#elif REAP_PLATFORM_WINDOWS || REAP_PLATFORM_LINUX
    constexpr int SYS_GL_CONTEXT_MAJOR = 4;
    constexpr int SYS_GL_CONTEXT_MINOR = 5;
    constexpr int SYS_GL_CONTEXT_FLAGS = 0; 
#else
    // Conservative fallback for unknown platforms.
    constexpr int SYS_GL_SYS_GL_CONTEXT_MAJOR = 4;
    constexpr int SYS_GL_SYS_GL_CONTEXT_MINOR = 1;
    constexpr int SYS_GL_CONTEXT_FLAGS = 0;
#endif

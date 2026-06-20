#ifndef CYPHER_ENGINE_SYSTEM_OPENGL_H
#define CYPHER_ENGINE_SYSTEM_OPENGL_H

#pragma once

#include "CypherSystem_Platform.h"

#include <glad/gl.h>       // OpenGL function declarations loaded by GLAD.
#include <SDL3/SDL.h>      // SDL_GL context attribute constants.

/*
================
OpenGL Platform Defaults

macOS is capped at OpenGL 4.1 core, while Windows/Linux can request newer GL.
================
*/
#ifdef CYPHER_PLATFORM_MACOS
    constexpr int SYS_GL_CONTEXT_MAJOR = 4;
    constexpr int SYS_GL_CONTEXT_MINOR = 1;
    constexpr int SYS_GL_CONTEXT_FLAGS = SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG;
#elif defined( CYPHER_PLATFORM_WINDOWS ) || defined( CYPHER_PLATFORM_LINUX )
    constexpr int SYS_GL_CONTEXT_MAJOR = 4;
    constexpr int SYS_GL_CONTEXT_MINOR = 5;
    constexpr int SYS_GL_CONTEXT_FLAGS = 0;
#else
    // Conservative fallback for unknown platforms.
    constexpr int SYS_GL_SYS_GL_CONTEXT_MAJOR = 4;
    constexpr int SYS_GL_SYS_GL_CONTEXT_MINOR = 1;
    constexpr int SYS_GL_CONTEXT_FLAGS = 0;
#endif

#endif // CYPHER_ENGINE_SYSTEM_OPENGL_H

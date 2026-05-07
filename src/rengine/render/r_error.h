#pragma once

#include "rengine/rcommon/com_error.h"
#include "rengine/rcommon/com_main.h"

namespace reap::rengine::render
{
    
enum class r_error_code_t : rcommon::u8 {
    OK = 0,

    ERR_IS_INIT,
    ERR_NOT_INIT,
    
    ERR_OPENGL_INIT,
    ERR_OPENGL_BEGIN_DRAW,
    ERR_OPENGL_END_DRAW,
    ERR_BEGIN_DRAW,
    ERR_END_DRAW,
    
    ERR_INVALID_WINDOW_CFG,
    ERR_INVALID_FUNC_PARAMETER,
    
    ERR_SHADER_REGISTRY_FULL,
    ERR_SHADER_LOAD,
    ERR_SHADER_COMPILE,
    ERR_SHADER_LINK,
    ERR_SHADER_BIND,
    
    ERR_INVALID_VIEWPORT,
    ERR_FRAME_ALREADY_ACTIVE,
    ERR_FRAME_NOT_ACTIVE,
};

constexpr inline const char *R_ErrorName( const r_error_code_t error ) {
    switch ( error ) {
    case r_error_code_t::OK:
        return "OK";
    case r_error_code_t::ERR_IS_INIT:
        return "ERR_IS_INIT";
    case r_error_code_t::ERR_NOT_INIT:
        return "ERR_NOT_INIT";
    case r_error_code_t::ERR_OPENGL_INIT:
        return "ERR_OPENGL_INIT";
    case r_error_code_t::ERR_OPENGL_BEGIN_DRAW:
        return "ERR_OPENGL_BEGIN_DRAW";
    case r_error_code_t::ERR_OPENGL_END_DRAW:
        return "ERR_OPENGL_END_DRAW";
    case r_error_code_t::ERR_SHADER_LOAD:
        return "ERR_SHADER_LOAD";
    case r_error_code_t::ERR_SHADER_COMPILE:
        return "ERR_SHADER_COMPILE";
    case r_error_code_t::ERR_SHADER_LINK:
        return "ERR_SHADER_LINK";
    case r_error_code_t::ERR_SHADER_BIND:
        return "ERR_SHADER_BIND";
    case r_error_code_t::ERR_BEGIN_DRAW:
        return "ERR_BEGIN_DRAW";
    case r_error_code_t::ERR_END_DRAW:
        return "ERR_END_DRAW";
    case r_error_code_t::ERR_INVALID_WINDOW_CFG:
        return "ERR_INVALID_WINDOW_CFG";
    case r_error_code_t::ERR_INVALID_FUNC_PARAMETER:
        return "ERR_INVALID_FUNC_PARAMETER";
    case r_error_code_t::ERR_SHADER_REGISTRY_FULL:
        return "ERR_SHADER_REGISTRY_FULL";
    case r_error_code_t::ERR_INVALID_VIEWPORT:
        return "ERR_INVALID_VIEWPORT";
    case r_error_code_t::ERR_FRAME_ALREADY_ACTIVE:
        return "ERR_FRAME_ALREADY_ACTIVE";
    case r_error_code_t::ERR_FRAME_NOT_ACTIVE:
        return "ERR_FRAME_NOT_ACTIVE";
    default:
        return "ERR_UNKNOWN";
    }
}

constexpr inline const char *R_ErrorDesc( const r_error_code_t error ) {
    switch ( error ) {
    case r_error_code_t::OK:
        return "operation completed successfully";
    case r_error_code_t::ERR_IS_INIT:
        return "render subsystem is already initialized";
    case r_error_code_t::ERR_NOT_INIT: 
        return "render subsystem is not initialized";
    case r_error_code_t::ERR_OPENGL_INIT:
        return "failed to initialize OpenGL context";
    case r_error_code_t::ERR_OPENGL_BEGIN_DRAW:
        return "failed to start OpenGL drawing";
    case r_error_code_t::ERR_OPENGL_END_DRAW:
        return "failed to end OpenGL drawing";
    case r_error_code_t::ERR_SHADER_LOAD:
        return "error loading shader program";
    case r_error_code_t::ERR_SHADER_COMPILE:
        return "error compiling a shader";
    case r_error_code_t::ERR_SHADER_LINK:
        return "error linking shader program";
    case r_error_code_t::ERR_SHADER_BIND:
        return "error binding shader program";
    case r_error_code_t::ERR_BEGIN_DRAW:
        return "failed to begin draw the frame";
    case r_error_code_t::ERR_END_DRAW:
        return "failed to end draw the frame";
    case r_error_code_t::ERR_INVALID_WINDOW_CFG:
        return "invalid render window configuration";
    case r_error_code_t::ERR_INVALID_FUNC_PARAMETER:
        return "invalid parameter passed to the function";
    case r_error_code_t::ERR_SHADER_REGISTRY_FULL:
        return "shader registry is full";
    case r_error_code_t::ERR_INVALID_VIEWPORT:
        return "invalid render viewport dimensions";
    case r_error_code_t::ERR_FRAME_ALREADY_ACTIVE:
        return "render frame is already active";
    case r_error_code_t::ERR_FRAME_NOT_ACTIVE:
        return "render frame is not active";
    default:
        return "unknown render error";
    }
}

constexpr inline rcommon::com_error_t R_ErrorCode( r_error_code_t code ) {
    return rcommon::Com_ErrorMake( rcommon::com_domain_t::COM_DOMAIN_RENDER , static_cast<rcommon::u16>( code ) );
}

}       // namespace reap::rengine::render

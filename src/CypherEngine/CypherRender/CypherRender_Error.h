#pragma once

#include "CypherEngine/CypherCommon/CypherCommon_Error.h"
#include "CypherEngine/CypherCommon/CypherCommon.h"

namespace cypher::engine::render
{

/*
================
Renderer Error Codes
================
*/
enum class error_code_t : common::u8 {
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
    ERR_SHADER_UNIFORM,
    ERR_DRAW_LIST_FULL,
    
    ERR_INVALID_VIEWPORT,
    ERR_FRAME_ALREADY_ACTIVE,
    ERR_FRAME_NOT_ACTIVE,
};

/*
================
Renderer Error Helpers
================
*/
constexpr inline const char *CypherRender_ErrorName( const error_code_t error ) {
    switch ( error ) {
    case error_code_t::OK:
        return "OK";
    case error_code_t::ERR_IS_INIT:
        return "ERR_IS_INIT";
    case error_code_t::ERR_NOT_INIT:
        return "ERR_NOT_INIT";
    case error_code_t::ERR_OPENGL_INIT:
        return "ERR_OPENGL_INIT";
    case error_code_t::ERR_OPENGL_BEGIN_DRAW:
        return "ERR_OPENGL_BEGIN_DRAW";
    case error_code_t::ERR_OPENGL_END_DRAW:
        return "ERR_OPENGL_END_DRAW";
    case error_code_t::ERR_SHADER_LOAD:
        return "ERR_SHADER_LOAD";
    case error_code_t::ERR_SHADER_COMPILE:
        return "ERR_SHADER_COMPILE";
    case error_code_t::ERR_SHADER_LINK:
        return "ERR_SHADER_LINK";
    case error_code_t::ERR_SHADER_BIND:
        return "ERR_SHADER_BIND";
    case error_code_t::ERR_SHADER_UNIFORM:
        return "ERR_SHADER_UNIFORM";
    case error_code_t::ERR_DRAW_LIST_FULL:
        return "ERR_DRAW_LIST_FULL";
    case error_code_t::ERR_BEGIN_DRAW:
        return "ERR_BEGIN_DRAW";
    case error_code_t::ERR_END_DRAW:
        return "ERR_END_DRAW";
    case error_code_t::ERR_INVALID_WINDOW_CFG:
        return "ERR_INVALID_WINDOW_CFG";
    case error_code_t::ERR_INVALID_FUNC_PARAMETER:
        return "ERR_INVALID_FUNC_PARAMETER";
    case error_code_t::ERR_SHADER_REGISTRY_FULL:
        return "ERR_SHADER_REGISTRY_FULL";
    case error_code_t::ERR_INVALID_VIEWPORT:
        return "ERR_INVALID_VIEWPORT";
    case error_code_t::ERR_FRAME_ALREADY_ACTIVE:
        return "ERR_FRAME_ALREADY_ACTIVE";
    case error_code_t::ERR_FRAME_NOT_ACTIVE:
        return "ERR_FRAME_NOT_ACTIVE";
    default:
        return "ERR_UNKNOWN";
    }
}

constexpr inline const char *CypherRender_ErrorDesc( const error_code_t error ) {
    switch ( error ) {
    case error_code_t::OK:
        return "operation completed successfully";
    case error_code_t::ERR_IS_INIT:
        return "render subsystem is already initialized";
    case error_code_t::ERR_NOT_INIT: 
        return "render subsystem is not initialized";
    case error_code_t::ERR_OPENGL_INIT:
        return "failed to initialize OpenGL context";
    case error_code_t::ERR_OPENGL_BEGIN_DRAW:
        return "failed to start OpenGL drawing";
    case error_code_t::ERR_OPENGL_END_DRAW:
        return "failed to end OpenGL drawing";
    case error_code_t::ERR_SHADER_LOAD:
        return "error loading shader program";
    case error_code_t::ERR_SHADER_COMPILE:
        return "error compiling a shader";
    case error_code_t::ERR_SHADER_LINK:
        return "error linking shader program";
    case error_code_t::ERR_SHADER_BIND:
        return "error binding shader program";
    case error_code_t::ERR_SHADER_UNIFORM:
        return "error setting shader uniform";
    case error_code_t::ERR_DRAW_LIST_FULL:
        return "draw list is full";
    case error_code_t::ERR_BEGIN_DRAW:
        return "failed to begin draw the frame";
    case error_code_t::ERR_END_DRAW:
        return "failed to end draw the frame";
    case error_code_t::ERR_INVALID_WINDOW_CFG:
        return "invalid render window configuration";
    case error_code_t::ERR_INVALID_FUNC_PARAMETER:
        return "invalid parameter passed to the function";
    case error_code_t::ERR_SHADER_REGISTRY_FULL:
        return "shader registry is full";
    case error_code_t::ERR_INVALID_VIEWPORT:
        return "invalid render viewport dimensions";
    case error_code_t::ERR_FRAME_ALREADY_ACTIVE:
        return "render frame is already active";
    case error_code_t::ERR_FRAME_NOT_ACTIVE:
        return "render frame is not active";
    default:
        return "unknown render error";
    }
}

constexpr inline common::error_t CypherRender_ErrorCode( error_code_t code ) {
    return common::CypherCommon_ErrorMake( common::domain_t::COM_DOMAIN_RENDER , static_cast<common::u16>( code ) );
}

}       // namespace cypher::engine::render

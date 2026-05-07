#pragma once

#include "rengine/rcommon/com_error.h"

#include <cstdarg>

namespace reap::rengine::rcommon
{

constexpr usize COM_MSG_MAX = 2048u;

void Com_Printf( const char *message, ... );

void Com_DPrintf( const char *message, ... );

void Com_VPrintf( const char *message, va_list args );

void Com_Errorf( const com_error_t error, const char *message, ... );

void Com_VErrorf( const com_error_t error, const char *message, va_list args );

}


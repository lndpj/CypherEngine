#ifndef CYPHER_COMMON_TIER0_STACKTRACE_H
#define CYPHER_COMMON_TIER0_STACKTRACE_H
#pragma once

/*
================
CypherCommon Stack Trace

Small placeholder API for future platform stack capture.
No allocation and no symbol loading at Tier0.
================
*/

#include "CypherCommon_BaseTypes.h"

namespace cypher::common
{

constexpr u32 CYPHER_STACK_TRACE_MAX_FRAMES = 64u;

struct stack_frame_t {
    void *address;
};

struct stack_trace_t {
    stack_frame_t frames[CYPHER_STACK_TRACE_MAX_FRAMES];
    u32 frame_count;
};

inline void ClearStackTrace( stack_trace_t &trace )
{
    trace.frame_count = 0u;
    for ( u32 i = 0u; i < CYPHER_STACK_TRACE_MAX_FRAMES; ++i ) {
        trace.frames[i].address = nullptr;
    }
}

inline u32 CaptureStackTrace( stack_trace_t &trace, u32 max_frames, u32 skip_frames )
{
    ( void )max_frames;
    ( void )skip_frames;
    ClearStackTrace( trace );
    return 0u;
}

} // namespace cypher::common

#endif // CYPHER_COMMON_TIER0_STACKTRACE_H

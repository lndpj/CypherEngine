#pragma once

#include "rengine/rcommon/com_main.h"
#include "rengine/sys/sys_window.h"

namespace reap::rengine::host
{

/*
================
Host Defaults
================
*/
constexpr rcommon::com_u32 HOST_DEFAULT_VIEWPORT_WIDTH  = 1280u;
constexpr rcommon::com_u32 HOST_DEFAULT_VIEWPORT_HEIGHT = 720u;
constexpr rcommon::com_u32 HOST_DEFAULT_TARGET_FPS      = 60u;    
constexpr const char *HOST_DEFAULT_WINDOW_TITLE         = "REAP";

/*
================
Host State Types

The host owns high-level engine lifecycle and keeps subsystem-facing config.
================
*/
enum class host_stage_t : rcommon::u8 {
    UNINITIALIZED,
    INITIALIZING,
    RUNNING,
    PAUSED,
    SHUTTINGDOWN,
    SHUTDOWN
};

enum class build_config_t : rcommon::u8 {
    UNKNOWN,
    DEBUG,
    RELEASE,
    DISTRIBUTION
};

/*
================
Host Runtime Data
================
*/
struct viewport_t {
    rcommon::com_u32 width{ HOST_DEFAULT_VIEWPORT_WIDTH };
    rcommon::com_u32 height{ HOST_DEFAULT_VIEWPORT_HEIGHT };
};

struct window_config_t {
    viewport_t viewport{};
    const char *title{ HOST_DEFAULT_WINDOW_TITLE };
    bool fullscreen{false};
    bool vsync{true};
    rcommon::com_u32 target_fps{ HOST_DEFAULT_TARGET_FPS };
};

struct frame_t {
    rcommon::com_frame_index_t index{};
    
    rcommon::f64 previous_time_seconds{};
    rcommon::f64 current_time_seconds{};
    
    rcommon::com_f32 delta_time_seconds{};
    rcommon::com_f32 real_time_seconds{};
    rcommon::com_f32 simulation_time_seconds{};
};

struct host_config_t {
    int argc{ 0 };
    const char *const *argv{ nullptr };
    
    build_config_t build_config{ build_config_t::UNKNOWN };
    window_config_t window_config{};
};

struct host_state_t {
    host_stage_t stage{ host_stage_t::UNINITIALIZED };
    host_config_t config{};
    sys::sys_window_t window{};
    bool running{ false };
    bool has_focus{ false };
    frame_t frame{};
};

}       // namespace reap::rengine::host

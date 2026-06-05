#pragma once

#include "CypherEngine/CypherCommon/CypherCommon.h"
#include "CypherEngine/CypherSystem/CypherSystem_Window.h"

namespace cypher::engine::host
{

/*
================
Host Defaults
================
*/
constexpr common::com_u32 HOST_DEFAULT_VIEWPORT_WIDTH  = 1280u;
constexpr common::com_u32 HOST_DEFAULT_VIEWPORT_HEIGHT = 720u;
constexpr common::com_u32 HOST_DEFAULT_TARGET_FPS      = 60u;    
constexpr const char *HOST_DEFAULT_WINDOW_TITLE         = "REAP";

/*
================
Host State Types

The host owns high-level engine lifecycle and keeps subsystem-facing config.
================
*/
enum class cypher_host_stage_t : common::u8 {
    UNINITIALIZED,
    INITIALIZING,
    RUNNING,
    PAUSED,
    SHUTTINGDOWN,
    SHUTDOWN
};

enum class build_config_t : common::u8 {
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
    common::com_u32 width{ HOST_DEFAULT_VIEWPORT_WIDTH };
    common::com_u32 height{ HOST_DEFAULT_VIEWPORT_HEIGHT };
};

struct window_config_t {
    viewport_t viewport{};
    const char *title{ HOST_DEFAULT_WINDOW_TITLE };
    bool fullscreen{false};
    bool vsync{true};
    common::com_u32 target_fps{ HOST_DEFAULT_TARGET_FPS };
};

struct frame_t {
    common::cypher_common_frame_index_t index{};
    
    common::f64 previous_time_seconds{};
    common::f64 current_time_seconds{};
    
    common::com_f32 delta_time_seconds{};
    common::com_f32 real_time_seconds{};
    common::com_f32 simulation_time_seconds{};
};

struct cypher_host_config_t {
    int argc{ 0 };
    const char *const *argv{ nullptr };
    
    build_config_t build_config{ build_config_t::UNKNOWN };
    window_config_t window_config{};
};

struct cypher_host_state_t {
    cypher_host_stage_t stage{ cypher_host_stage_t::UNINITIALIZED };
    cypher_host_config_t config{};
    sys::cypher_system_window_t window{};
    bool running{ false };
    bool has_focus{ false };
    frame_t frame{};
};

}       // namespace cypher::engine::host

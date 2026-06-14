#ifndef CYPHER_ENGINE_MEMORY_H
#define CYPHER_ENGINE_MEMORY_H

#pragma once

#include "CypherEngine/CypherCommon/CypherCommon.h"
#include "CypherEngine/CypherMemory/CypherMemory_Arena.h"
#include "CypherEngine/CypherMemory/CypherMemory_Pool.h"
#include "CypherEngine/CypherMemory/CypherMemory_Scratch.h"
#include "CypherEngine/CypherMemory/CypherMemory_Bucket.h"
#include "CypherEngine/CypherMemory/CypherMemory_Thread.h"

namespace cypher::engine::memory
{
 
enum class memory_tag_t : common::u8 {
    UNKNOWN = 0,
    CORE,
    SYSTEM,
    MEMORY,
    FILESYSTEM,
    RESOURCE,
    WORLD,
    RENDER,
    AUDIO,
    PHYSICS,
    AI,
    SCRIPT,
    NETWORK,
    EDITOR,
    TOOLS,
    TEMP,
    COUNT 
};

struct memory_tag_stats_t {
    const char *name{ nullptr };
    common::usize used{ 0u };
    common::usize peak_used{ 0u };
    common::u64 allocation_count{ 0u };
    common::u64 failed_allocation_count{ 0u };
};
 
struct memory_arena_config_t {
    const char *name{ nullptr };
    common::usize reserve_size{ 0u };
    common::usize initial_commit{ 0u };
    common::u32 flags{ CYPHER_MEMORY_ARENA_FLAG_NONE };
    arena_backing_t backing{ arena_backing_t::ARENA_VIRTUAL_MEMORY };
    memory_tag_t tag{ memory_tag_t::MEMORY };
};

struct memory_config_t {
    memory_arena_config_t permanent_arena;
    memory_arena_config_t frame_arena;
    memory_arena_config_t scratch_arena;
    memory_arena_config_t resource_arena;
    memory_arena_config_t world_arena;
    memory_arena_config_t render_arena;
    memory_arena_config_t editor_arena;
};

struct memory_stats_t {
    common::usize total_capacity{ 0u };
    common::usize total_committed{ 0u };
    common::usize total_used{ 0u };
    common::usize peak_used{ 0u };

    arena_stats_t permanent_stats{};
    arena_stats_t frame_stats{};
    arena_stats_t scratch_stats{};
    arena_stats_t resource_stats{};
    arena_stats_t world_stats{};
    arena_stats_t render_stats{};
    arena_stats_t editor_stats{};

    memory_tag_stats_t tag_stats[static_cast<common::usize>( memory_tag_t::COUNT )]{};
};

struct memory_state_t {
    bool initialized{ false };

    memory_config_t config{};

    arena_t permanent_arena{};
    arena_t frame_arena{};
    arena_t scratch_arena{};
    arena_t resource_arena{};
    arena_t world_arena{};
    arena_t render_arena{};
    arena_t editor_arena{};

    common::usize total_capacity{ 0u };
    common::usize total_committed{ 0u };
    common::usize total_used{ 0u };
    common::usize peak_used{ 0u };
};

memory_config_t CypherMemory_DefaultConfig();

mem_error_t CypherMemory_Init( const memory_config_t &config );

void CypherMemory_Shutdown();

void CypherMemory_BeginFrame();

void CypherMemory_EndFrame();

bool CypherMemory_IsInitialized();

memory_stats_t CypherMemory_Stats();

const char *CypherMemory_TagName( memory_tag_t tag );

arena_t &CypherMemory_PermanentArena();

arena_t &CypherMemory_FrameArena();

arena_t &CypherMemory_ScratchArena();

arena_t &CypherMemory_ResourceArena();

arena_t &CypherMemory_WorldArena();

arena_t &CypherMemory_RenderArena();

arena_t &CypherMemory_EditorArena();

}       // namespace cypher::engine::memory

#endif // CYPHER_ENGINE_MEMORY_H

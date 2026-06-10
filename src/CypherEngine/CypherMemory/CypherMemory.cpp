/*======================================================================
   File: CypherMemory.cpp
   Project: CYPHER
   Author: ksiric <email@example.com>
   Created: 2026-06-10 11:16:54
   Last Modified by: ksiric
   Last Modified: 2026-06-10 17:22:48
   ---------------------------------------------------------------------
   Description:
       
   ---------------------------------------------------------------------
   License: 
   Company: 
   Version: 0.1.0
 ======================================================================
                                                                       */
#include "CypherEngine/CypherMemory/CypherMemory.h"
#include "CypherEngine/CypherLog/CypherLog.h"

namespace {

cypher::engine::memory::memory_state_t g_memory{};

cypher::engine::memory::arena_desc_t CypherMemory_MakeArenaDesc( const cypher::engine::memory::memory_arena_config_t &arena_config )
{
    cypher::engine::memory::arena_desc_t desc{};

    desc.name = arena_config.name;
    desc.capacity = arena_config.reserve_size;
    desc.initial_commit = arena_config.initial_commit;
    desc.flags = arena_config.flags;
    desc.backing = arena_config.backing;

    return desc;
}

cypher::engine::memory::error_code_t CypherMemory_InitArena(
    cypher::engine::memory::arena_t &arena,
    const cypher::engine::memory::memory_arena_config_t &arena_config )
{
    return cypher::engine::memory::CypherMemory_ArenaInit(
        arena,
        CypherMemory_MakeArenaDesc( arena_config ) );
}

void CypherMemory_ShutdownInitializedArenas()
{
    cypher::engine::memory::CypherMemory_ArenaShutdown( g_memory.editor_arena );
    cypher::engine::memory::CypherMemory_ArenaShutdown( g_memory.render_arena );
    cypher::engine::memory::CypherMemory_ArenaShutdown( g_memory.world_arena );
    cypher::engine::memory::CypherMemory_ArenaShutdown( g_memory.resource_arena );
    cypher::engine::memory::CypherMemory_ArenaShutdown( g_memory.scratch_arena );
    cypher::engine::memory::CypherMemory_ArenaShutdown( g_memory.frame_arena );
    cypher::engine::memory::CypherMemory_ArenaShutdown( g_memory.permanent_arena );
}

void CypherMemory_AddArenaStats(
    const cypher::engine::memory::arena_t &arena,
    cypher::engine::memory::memory_stats_t &stats,
    const cypher::engine::memory::memory_tag_t tag )
{
    const cypher::engine::common::usize tag_index = static_cast<cypher::engine::common::usize>( tag );

    stats.total_capacity += arena.capacity;
    stats.total_committed += arena.committed;
    stats.total_used += arena.used;

    if ( arena.peak_used > stats.peak_used ) {
        stats.peak_used = arena.peak_used;
    }

    if ( tag_index < static_cast<cypher::engine::common::usize>( cypher::engine::memory::memory_tag_t::COUNT ) ) {
        stats.tag_stats[tag_index].name = cypher::engine::memory::CypherMemory_TagName( tag );
        stats.tag_stats[tag_index].used += arena.used;
        stats.tag_stats[tag_index].peak_used += arena.peak_used;
        stats.tag_stats[tag_index].allocation_count += arena.allocation_count;
        stats.tag_stats[tag_index].failed_allocation_count += arena.failed_allocation_count;
    }
}

}       // namespace

namespace cypher::engine::memory
{

memory_config_t CypherMemory_DefaultConfig()
{
    memory_config_t config{};

    config.permanent_arena = memory_arena_config_t{
        "PermanentArena",
        CypherMemory_Megabytes( 256u ),
        CypherMemory_Megabytes( 16u ),
        CYPHER_MEMORY_ARENA_FLAG_GROW_COMMIT_ON_ALLOC,
        arena_backing_t::ARENA_VIRTUAL_MEMORY,
        memory_tag_t::CORE
    };

    config.frame_arena = memory_arena_config_t{
        "FrameArena",
        CypherMemory_Megabytes( 64u ),
        CypherMemory_Megabytes( 8u ),
        CYPHER_MEMORY_ARENA_FLAG_GROW_COMMIT_ON_ALLOC | CYPHER_MEMORY_ARENA_FLAG_CLEAR_ON_RESET | CYPHER_MEMORY_ARENA_FLAG_DECOMMIT_ON_RESET,
        arena_backing_t::ARENA_VIRTUAL_MEMORY,
        memory_tag_t::TEMP
    };

    config.scratch_arena = memory_arena_config_t{
        "ScratchArena",
        CypherMemory_Megabytes( 128u ),
        CypherMemory_Megabytes( 8u ),
        CYPHER_MEMORY_ARENA_FLAG_GROW_COMMIT_ON_ALLOC | CYPHER_MEMORY_ARENA_FLAG_DECOMMIT_ON_RESET,
        arena_backing_t::ARENA_VIRTUAL_MEMORY,
        memory_tag_t::TEMP
    };

    config.resource_arena = memory_arena_config_t{
        "ResourceArena",
        CypherMemory_Gigabytes( 2u ),
        CypherMemory_Megabytes( 64u ),
        CYPHER_MEMORY_ARENA_FLAG_GROW_COMMIT_ON_ALLOC,
        arena_backing_t::ARENA_VIRTUAL_MEMORY,
        memory_tag_t::RESOURCE
    };

    config.world_arena = memory_arena_config_t{
        "WorldArena",
        CypherMemory_Gigabytes( 1u ),
        CypherMemory_Megabytes( 64u ),
        CYPHER_MEMORY_ARENA_FLAG_GROW_COMMIT_ON_ALLOC,
        arena_backing_t::ARENA_VIRTUAL_MEMORY,
        memory_tag_t::WORLD
    };

    config.render_arena = memory_arena_config_t{
        "RenderArena",
        CypherMemory_Megabytes( 512u ),
        CypherMemory_Megabytes( 32u ),
        CYPHER_MEMORY_ARENA_FLAG_GROW_COMMIT_ON_ALLOC,
        arena_backing_t::ARENA_VIRTUAL_MEMORY,
        memory_tag_t::RENDER
    };

    config.editor_arena = memory_arena_config_t{
        "EditorArena",
        CypherMemory_Megabytes( 512u ),
        CypherMemory_Megabytes( 32u ),
        CYPHER_MEMORY_ARENA_FLAG_GROW_COMMIT_ON_ALLOC,
        arena_backing_t::ARENA_VIRTUAL_MEMORY,
        memory_tag_t::EDITOR
    };

    return config;
}

error_code_t CypherMemory_Init( const memory_config_t &config )
{
    if ( g_memory.initialized ) {
        CYPHER_LOG_WARNING( log::channel_t::MEMORY, "memory system is already initialized." );
        return error_code_t::ERR_ALREADY_INITIALIZED;
    }

    g_memory = {};
    g_memory.config = config;

    error_code_t result = CypherMemory_InitArena( g_memory.permanent_arena, config.permanent_arena );
    if ( result != error_code_t::OK ) {
        CypherMemory_ShutdownInitializedArenas();
        g_memory = {};
        return result;
    }

    result = CypherMemory_InitArena( g_memory.frame_arena, config.frame_arena );
    if ( result != error_code_t::OK ) {
        CypherMemory_ShutdownInitializedArenas();
        g_memory = {};
        return result;
    }

    result = CypherMemory_InitArena( g_memory.scratch_arena, config.scratch_arena );
    if ( result != error_code_t::OK ) {
        CypherMemory_ShutdownInitializedArenas();
        g_memory = {};
        return result;
    }

    result = CypherMemory_InitArena( g_memory.resource_arena, config.resource_arena );
    if ( result != error_code_t::OK ) {
        CypherMemory_ShutdownInitializedArenas();
        g_memory = {};
        return result;
    }

    result = CypherMemory_InitArena( g_memory.world_arena, config.world_arena );
    if ( result != error_code_t::OK ) {
        CypherMemory_ShutdownInitializedArenas();
        g_memory = {};
        return result;
    }

    result = CypherMemory_InitArena( g_memory.render_arena, config.render_arena );
    if ( result != error_code_t::OK ) {
        CypherMemory_ShutdownInitializedArenas();
        g_memory = {};
        return result;
    }

    result = CypherMemory_InitArena( g_memory.editor_arena, config.editor_arena );
    if ( result != error_code_t::OK ) {
        CypherMemory_ShutdownInitializedArenas();
        g_memory = {};
        return result;
    }

    g_memory.initialized = true;

    const memory_stats_t stats = CypherMemory_Stats();
    CYPHER_LOG_INFO( log::channel_t::MEMORY,
                     "memory system initialized: capacity=%zu bytes, committed=%zu bytes.",
                     stats.total_capacity,
                     stats.total_committed );

    return error_code_t::OK;
}

void CypherMemory_Shutdown()
{
    if ( !g_memory.initialized ) {
        return;
    }

    const memory_stats_t stats = CypherMemory_Stats();
    CYPHER_LOG_INFO( log::channel_t::MEMORY,
                     "memory system shutdown: used=%zu bytes, peak=%zu bytes.",
                     stats.total_used,
                     stats.peak_used );

    CypherMemory_ShutdownInitializedArenas();
    g_memory = {};
}

void CypherMemory_BeginFrame()
{
    if ( !g_memory.initialized ) {
        return;
    }

    CypherMemory_ArenaReset( g_memory.frame_arena );
}

void CypherMemory_EndFrame()
{
    if ( !g_memory.initialized ) {
        return;
    }

    CypherMemory_ArenaResetCounters( g_memory.frame_arena );
}

bool CypherMemory_IsInitialized()
{
    return g_memory.initialized;
}

memory_stats_t CypherMemory_Stats()
{
    memory_stats_t stats{};

    stats.permanent_stats = CypherMemory_ArenaStats( g_memory.permanent_arena );
    stats.frame_stats = CypherMemory_ArenaStats( g_memory.frame_arena );
    stats.scratch_stats = CypherMemory_ArenaStats( g_memory.scratch_arena );
    stats.resource_stats = CypherMemory_ArenaStats( g_memory.resource_arena );
    stats.world_stats = CypherMemory_ArenaStats( g_memory.world_arena );
    stats.render_stats = CypherMemory_ArenaStats( g_memory.render_arena );
    stats.editor_stats = CypherMemory_ArenaStats( g_memory.editor_arena );

    CypherMemory_AddArenaStats( g_memory.permanent_arena, stats, g_memory.config.permanent_arena.tag );
    CypherMemory_AddArenaStats( g_memory.frame_arena, stats, g_memory.config.frame_arena.tag );
    CypherMemory_AddArenaStats( g_memory.scratch_arena, stats, g_memory.config.scratch_arena.tag );
    CypherMemory_AddArenaStats( g_memory.resource_arena, stats, g_memory.config.resource_arena.tag );
    CypherMemory_AddArenaStats( g_memory.world_arena, stats, g_memory.config.world_arena.tag );
    CypherMemory_AddArenaStats( g_memory.render_arena, stats, g_memory.config.render_arena.tag );
    CypherMemory_AddArenaStats( g_memory.editor_arena, stats, g_memory.config.editor_arena.tag );

    g_memory.total_capacity = stats.total_capacity;
    g_memory.total_committed = stats.total_committed;
    g_memory.total_used = stats.total_used;

    if ( stats.total_used > g_memory.peak_used ) {
        g_memory.peak_used = stats.total_used;
    }

    stats.peak_used = g_memory.peak_used;

    return stats;
}

const char *CypherMemory_TagName( const memory_tag_t tag )
{
    switch ( tag ) {
    case memory_tag_t::UNKNOWN:    return "UNKNOWN";
    case memory_tag_t::CORE:       return "CORE";
    case memory_tag_t::SYSTEM:     return "SYSTEM";
    case memory_tag_t::MEMORY:     return "MEMORY";
    case memory_tag_t::FILESYSTEM: return "FILESYSTEM";
    case memory_tag_t::RESOURCE:   return "RESOURCE";
    case memory_tag_t::WORLD:      return "WORLD";
    case memory_tag_t::RENDER:     return "RENDER";
    case memory_tag_t::AUDIO:      return "AUDIO";
    case memory_tag_t::PHYSICS:    return "PHYSICS";
    case memory_tag_t::AI:         return "AI";
    case memory_tag_t::SCRIPT:     return "SCRIPT";
    case memory_tag_t::NETWORK:    return "NETWORK";
    case memory_tag_t::EDITOR:     return "EDITOR";
    case memory_tag_t::TOOLS:      return "TOOLS";
    case memory_tag_t::TEMP:       return "TEMP";
    case memory_tag_t::COUNT:      return "COUNT";
    default:                       return "UNKNOWN";
    }
}

arena_t &CypherMemory_PermanentArena()
{
    return g_memory.permanent_arena;
}

arena_t &CypherMemory_FrameArena()
{
    return g_memory.frame_arena;
}

arena_t &CypherMemory_ScratchArena()
{
    return g_memory.scratch_arena;
}

arena_t &CypherMemory_ResourceArena()
{
    return g_memory.resource_arena;
}

arena_t &CypherMemory_WorldArena()
{
    return g_memory.world_arena;
}

arena_t &CypherMemory_RenderArena()
{
    return g_memory.render_arena;
}

arena_t &CypherMemory_EditorArena()
{
    return g_memory.editor_arena;
}

}       // namespace cypher::engine::memory

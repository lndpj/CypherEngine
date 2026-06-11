/*======================================================================
   File: CypherMemory_Scratch.cpp
   Project: CYPHER
   Author: ksiric <email@example.com>
   Created: 2026-06-12
   ---------------------------------------------------------------------
   Description:
       Scratch allocation scopes built on arena markers.
   ---------------------------------------------------------------------
   License:
   Company:
   Version: 0.1.0
 ======================================================================
                                                                       */

#include "CypherEngine/CypherMemory/CypherMemory_Scratch.h"
#include "CypherEngine/CypherLog/CypherLog.h"

namespace cypher::engine::memory
{

namespace {

void *CypherMemory_ScratchFailAlloc( scratch_scope_t &scope,
                                     const error_code_t error,
                                     const char *reason )
{
    scope.last_error = error;

    CYPHER_LOG_ERROR( log::channel_t::MEMORY,
                      "scratch scope '%s' allocation failed: %s.",
                      scope.name ? scope.name : "<unnamed>",
                      reason ? reason : CypherMemory_ErrorDesc( error ) );

    return nullptr;
}

}       // namespace

error_code_t CypherMemory_ScratchBegin( scratch_scope_t &scope, arena_t &arena, const char *name )
{
    if ( scope.active ) {
        scope.last_error = error_code_t::ERR_ALREADY_INITIALIZED;
        CYPHER_LOG_WARNING( log::channel_t::MEMORY,
                            "scratch scope '%s' begin failed: scope is already active.",
                            scope.name ? scope.name : "<unnamed>" );
        return scope.last_error;
    }

    if ( !CypherMemory_ArenaIsInitialized( arena ) ) {
        scope = {};
        scope.name = name;
        scope.arena = &arena;
        scope.last_error = error_code_t::ERR_NOT_INITIALIZED;
        CYPHER_LOG_ERROR( log::channel_t::MEMORY,
                          "scratch scope '%s' begin failed: backing arena is not initialized.",
                          name ? name : "<unnamed>" );
        return scope.last_error;
    }

    scope = {};
    scope.name = name;
    scope.arena = &arena;
    scope.marker = CypherMemory_ArenaGetMarker( arena );
    scope.used_at_begin = arena.used;
    scope.allocation_count_at_begin = arena.allocation_count;
    scope.failed_allocation_count_at_begin = arena.failed_allocation_count;
    scope.last_error = error_code_t::OK;
    scope.active = true;

    return error_code_t::OK;
}

error_code_t CypherMemory_ScratchEnd( scratch_scope_t &scope )
{
    if ( !scope.active ) {
        scope.last_error = error_code_t::ERR_NOT_INITIALIZED;
        return scope.last_error;
    }

    if ( scope.arena == nullptr || !CypherMemory_ArenaIsInitialized( *scope.arena ) ) {
        scope.last_error = error_code_t::ERR_NOT_INITIALIZED;
        scope.active = false;
        return scope.last_error;
    }

    const error_code_t rewind_result = CypherMemory_ArenaRewind( *scope.arena, scope.marker );
    scope.last_error = rewind_result;

    const char *name = scope.name;
    scope = {};
    scope.name = name;

    return rewind_result;
}

scratch_stats_t CypherMemory_ScratchStats( const scratch_scope_t &scope )
{
    scratch_stats_t stats{};

    stats.name = scope.name;
    stats.used_at_begin = scope.used_at_begin;
    stats.active = scope.active;

    if ( !scope.active || scope.arena == nullptr || !CypherMemory_ArenaIsInitialized( *scope.arena ) ) {
        return stats;
    }

    stats.used_current = scope.arena->used;
    stats.used_since_begin = stats.used_current >= scope.used_at_begin ? stats.used_current - scope.used_at_begin : 0u;
    stats.capacity = scope.arena->capacity;
    stats.allocation_count_since_begin = scope.arena->allocation_count >= scope.allocation_count_at_begin
                                             ? scope.arena->allocation_count - scope.allocation_count_at_begin
                                             : 0u;
    stats.failed_allocation_count_since_begin = scope.arena->failed_allocation_count >= scope.failed_allocation_count_at_begin
                                                    ? scope.arena->failed_allocation_count - scope.failed_allocation_count_at_begin
                                                    : 0u;

    return stats;
}

void *CypherMemory_ScratchAlloc( scratch_scope_t &scope, common::usize size, common::usize alignment )
{
    return CypherMemory_ScratchAllocDebug( scope, size, alignment, nullptr, nullptr, 0 );
}

void *CypherMemory_ScratchAllocDebug( scratch_scope_t &scope,
                                      common::usize size,
                                      common::usize alignment,
                                      const char *file,
                                      const char *function,
                                      common::i32 line )
{
    if ( !scope.active ) {
        return CypherMemory_ScratchFailAlloc( scope, error_code_t::ERR_NOT_INITIALIZED, "scope is not active" );
    }

    if ( scope.arena == nullptr || !CypherMemory_ArenaIsInitialized( *scope.arena ) ) {
        return CypherMemory_ScratchFailAlloc( scope, error_code_t::ERR_NOT_INITIALIZED, "backing arena is not initialized" );
    }

    void *memory = CypherMemory_ArenaAllocDebug( *scope.arena, size, alignment, file, function, line );
    scope.last_error = CypherMemory_ArenaLastError( *scope.arena );

    return memory;
}

void *CypherMemory_ScratchAllocZero( scratch_scope_t &scope, common::usize size, common::usize alignment )
{
    return CypherMemory_ScratchAllocZeroDebug( scope, size, alignment, nullptr, nullptr, 0 );
}

void *CypherMemory_ScratchAllocZeroDebug( scratch_scope_t &scope,
                                          common::usize size,
                                          common::usize alignment,
                                          const char *file,
                                          const char *function,
                                          common::i32 line )
{
    if ( !scope.active ) {
        return CypherMemory_ScratchFailAlloc( scope, error_code_t::ERR_NOT_INITIALIZED, "scope is not active" );
    }

    if ( scope.arena == nullptr || !CypherMemory_ArenaIsInitialized( *scope.arena ) ) {
        return CypherMemory_ScratchFailAlloc( scope, error_code_t::ERR_NOT_INITIALIZED, "backing arena is not initialized" );
    }

    void *memory = CypherMemory_ArenaAllocZeroDebug( *scope.arena, size, alignment, file, function, line );
    scope.last_error = CypherMemory_ArenaLastError( *scope.arena );

    return memory;
}

bool CypherMemory_ScratchIsActive( const scratch_scope_t &scope )
{
    return scope.active;
}

error_code_t CypherMemory_ScratchLastError( const scratch_scope_t &scope )
{
    return scope.last_error;
}

}       // namespace cypher::engine::memory

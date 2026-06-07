/*======================================================================
   File: CypherMemory_Arena.cpp
   Project: CYPHER
   Author: ksiric <email@example.com>
   Created: 2026-06-07 12:32:50
   Last Modified by: ksiric
   Last Modified: 2026-06-07 14:00:09
   ---------------------------------------------------------------------
   Description:
       
   ---------------------------------------------------------------------
   License: 
   Company: 
   Version: 0.1.0
 ======================================================================
                                                                       */

#include "CypherEngine/CypherMemory/CypherMemory_Arena.h"

#include <cstdlib>          // for malloc func
#include <cstring>
#include <new>              // for operator new

namespace cypher::engine::memory
{

error_code_t CypherMemory_ArenaInit( arena_t &arena, const arena_desc_t &arena_desc )
{
    if ( arena.initialized ) {
        arena.last_error = error_code_t::ERR_ALREADY_INITIALIZED;
        return error_code_t::ERR_ALREADY_INITIALIZED;
    }
    if ( arena_desc.capacity == 0u ) {
        arena.last_error = error_code_t::ERR_INVALID_CAPACITY;
        return error_code_t::ERR_INVALID_CAPACITY;
    }
    
    // allocate new physical memory space -> for virtual we reserve and then commit in pages.
    // @NOTE: Change once we write the virtual memory arena addressing.
    void *memory = ::operator new( arena_desc.capacity, std::nothrow );
    if ( memory == nullptr ) {
        arena.last_error = error_code_t::ERR_MEMORY_ALLOCATION;
        return error_code_t::ERR_MEMORY_ALLOCATION;
    }
    
    arena.name = arena_desc.name;
    arena.base = static_cast<common::u8 *>( memory );
    
    arena.capacity = arena_desc.capacity;
    arena.used = 0u;
    arena.peak_used = 0u;
    
    arena.committed = arena_desc.capacity;
    arena.page_size = 0u;
    
    arena.allocation_count = 0u;
    arena.failed_allocation_count = 0u;
    
    arena.flags = arena_desc.flags;
    arena.last_error = error_code_t::OK;
    
    arena.backing = arena_desc.backing;
    
    arena.initialized = true;
    arena.owns_memory = true;
    
    return error_code_t::OK;
}

void CypherMemory_ArenaShutdown( arena_t &arena )
{
    // @NOTE: Change once we implement the virtual memory paging and addressing.
    if ( !arena.initialized ) {
        return ;
    }
    
    if ( arena.owns_memory && arena.base != nullptr ) {
        ::operator delete( arena.base );
    }
    
    arena = {};
}

void CypherMemory_ArenaReset( arena_t &arena )
{
    // Only resets the arena memory does not free the memory itself.
    if ( !arena.initialized ) {
        return ;
    }
    
    if  ( ( arena.flags & CYPHER_MEMORY_ARENA_FLAG_CLEAR_ON_RESET ) != 0 ) {
        std::memset( arena.base, 0, arena.used );
    }
    
    arena.used = 0u;
    arena.last_error = error_code_t::OK;
}

arena_stats_t CypherMemory_ArenaStats( const arena_t &arena )
{
    arena_stats_t stats{};
    
    stats.name = arena.name;
    stats.allocation_count = arena.allocation_count;
    stats.failed_allocation_count = arena.failed_allocation_count;
    stats.capacity = arena.capacity;
    stats.peak_used = arena.peak_used;
    stats.remaining = arena.capacity - arena.used;
    stats.used = arena.used;   
    
    return stats;
}

void CypherMemory_ArenaResetCounters( arena_t &arena ) 
{
    if ( !arena.initialized ) {
        return ;
    }
    
    arena.peak_used = arena.used;
    arena.allocation_count = 0u;
    arena.failed_allocation_count = 0u;
    arena.last_error = error_code_t::OK;
}
    
}       // namespace cypher::engine::memory

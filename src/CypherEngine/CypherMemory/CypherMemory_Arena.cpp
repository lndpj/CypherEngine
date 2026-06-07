/*======================================================================
   File: CypherMemory_Arena.cpp
   Project: CYPHER
   Author: ksiric <email@example.com>
   Created: 2026-06-07 12:32:50
   Last Modified by: ksiric
   Last Modified: 2026-06-07 22:14:46
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
#include <cstring>          // for string memset
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

void *CypherMemory_ArenaAlloc( arena_t &arena, common::usize size, common::usize alignment )
{
    if ( !arena.initialized ) {
        arena.last_error = error_code_t::ERR_NOT_INITIALIZED;
        ++arena.failed_allocation_count;
        return nullptr;
    }
    if ( size == 0u ) {
        arena.last_error = error_code_t::ERR_INVALID_ARGUMENT;
        ++arena.failed_allocation_count;
        return nullptr;
    }
    
    if ( !CypherMemory_IsPowerOfTwo( alignment ) ) {
        arena.last_error = error_code_t::ERR_INVALID_ALIGNMENT;
        ++arena.failed_allocation_count;
        return nullptr;
    } 
    
    const common::usize current_address = reinterpret_cast<common::usize>( arena.base ) + arena.used;
    const common::usize aligned_address = CypherMemory_AlignForward( current_address, alignment );
    const common::usize padding         = aligned_address - current_address;
    const common::usize new_address     = arena.used + padding + size;
    
    if ( new_address > arena.capacity ) {
        arena.last_error = error_code_t::ERR_OUT_OF_MEMORY;
        ++arena.failed_allocation_count;
        return nullptr;
    }
    
    arena.used = new_address;
    
    if ( arena.used > arena.peak_used ) {
        arena.peak_used = arena.used;
    }
    
    ++arena.allocation_count;
    arena.last_error = error_code_t::OK;
    
    void *result = reinterpret_cast<void *>( aligned_address );
    
    if ( ( arena.flags & CYPHER_MEMORY_ARENA_FLAG_ZERO_ON_ALLOC ) != 0 ) {
        std::memset( result, 0, size );
    }
    
    return result;
}

void *CypherMemory_ArenaAllocZero( arena_t &arena, common::usize size, common::usize alignment )
{
    void *memory = CypherMemory_ArenaAlloc( arena, size, alignment );
    
    if ( memory == nullptr ) {
        return nullptr;
    }
    std::memset( memory, 0, size );
    
    return memory;
}

common::usize CypherMemory_ArenaRemaining( const arena_t &arena )
{
    if ( !arena.initialized ) {
        return 0u;
    }
    
    if ( arena.used >= arena.capacity ) {
        return 0u;
    }
    
    return arena.capacity - arena.used;
}

arena_marker_t CypherMemory_ArenaGetMarker( const arena_t &arena )
{
    arena_marker_t marker{};
    
    if ( !arena.initialized ) {
        return marker;
    }
    
    marker.used = arena.used;
    return marker;
}

error_code_t CypherMemory_ArenaRewind( arena_t &arena, arena_marker_t marker )
{
    if ( !arena.initialized ) {
        arena.last_error = error_code_t::ERR_NOT_INITIALIZED;
        return error_code_t::ERR_NOT_INITIALIZED;
    }
    
    if ( marker.used > arena.used || marker.used > arena.capacity ) {
        arena.last_error = error_code_t::ERR_INVALID_MARKER;
        return error_code_t::ERR_INVALID_MARKER;
    }
    
    if ( ( arena.flags & CYPHER_MEMORY_ARENA_FLAG_CLEAR_ON_RESET ) != 0u ) {
        std::memset( arena.base + marker.used,
                     0,
                     arena.used - marker.used );
    }
    
    arena.used = marker.used;
    arena.last_error = error_code_t::OK;
    
    return error_code_t::OK;
}

bool CypherMemory_ArenaContains( const arena_t &arena, const void *ptr )
{
    if ( !arena.initialized || arena.base == nullptr || ptr == nullptr ) {
        return false;
    }   
    
    const common::usize address = reinterpret_cast<common::usize>( ptr );
    const common::usize base    = reinterpret_cast<common::usize>( arena.base );
    const common::usize end     = base + arena.capacity;
    
    return address >= base && address < end;
}

error_code_t CypherMemory_ArenaLastError( const arena_t &arena )
{
    return arena.last_error;
}

bool CypherMemory_ArenaIsInitialized( const arena_t &arena )
{
    return arena.initialized;
}

common::usize CypherMemory_ArenaUsed( const arena_t &arena )
{
    return arena.used;
}

common::f32 CypherMemory_ArenaUsageRatio( const arena_t &arena )
{
    if ( arena.capacity == 0u ) {
        return 0.0f;
    }
    return static_cast<common::f32>( arena.used ) / static_cast<common::f32>( arena.capacity );
}

common::usize CypherMemory_ArenaCapacity( const arena_t &arena )
{
    
}


template <typename T>
T *CypherMemory_ArenaAllocType( arena_t &arena ) {
    return static_cast<T *>(
            CypherMemory_ArenaAlloc(
            arena,
            sizeof( T ),
            alignof( T ) ) );   
}

template <typename T>
T *CypherMemory_ArenaAllocArray( arena_t &arena, const common::usize count )
{
    return static_cast<T *>(
            CypherMemory_ArenaAlloc(
            arena,
            sizeof( T ) * count,
            alignof( T ) ) );   
}

template <typename T>
T *CypherMemory_ArenaAllocArrayZero( arena_t &arena, const common::usize count )
{
    return static_cast<T *>(
            CypherMemory_ArenaAllocZero(
            arena,
            sizeof( T ) * count,
            alignof( T ) ) );   
}

template <typename T>
T *CypherMemory_ArenaAllocTypeZero( arena_t &arena )
{
        return static_cast<T *>(
            CypherMemory_ArenaAllocZero(
            arena,
            sizeof( T ),
            alignof( T ) ) );   
}

}       // namespace cypher::engine::memory

/*======================================================================
   File: CypherMemory_Arena.cpp
   Project: CYPHER
   Author: ksiric <email@example.com>
   Created: 2026-06-07 12:32:50
   Last Modified by: ksiric
   Last Modified: 2026-06-07 23:39:18
   ---------------------------------------------------------------------
   Description:
       
   ---------------------------------------------------------------------
   License: 
   Company: 
   Version: 0.1.0
 ======================================================================
                                                                       */

#include "CypherEngine/CypherMemory/CypherMemory_Arena.h"
#include "CypherEngine/CypherLog/CypherLog.h"
#include "CypherEngine/CypherSystem/CypherSystem_Platform.h"

#include <cstdlib>          // for malloc func
#include <cstring>          // for string memset
#include <new>              // for operator new

namespace cypher::engine::memory
{

namespace {

/*
================
CypherMemory_AlignSizeForward
================
*/
common::usize CypherMemory_AlignSizeForward( const common::usize size, const common::usize alignment )
{
    if ( size == 0u || alignment == 0u ) {
        return size;
    }

    common::usize aligned_size = size;
    if ( !CypherMemory_AlignForwardChecked( size, alignment, aligned_size ) ) {
        return 0u;
    }

    return aligned_size;
}

/*
================
CypherMemory_ArenaRecordAllocationTrace
================
*/
void CypherMemory_ArenaRecordAllocationTrace(
    arena_t &arena,
    void *ptr,
    const common::usize size,
    const common::usize alignment,
    const char *file,
    const char *function,
    const common::i32 line,
    const error_code_t error,
    const bool failed )
{
    const common::usize trace_index = arena.allocation_trace_index % CYPHER_MEMORY_ARENA_ALLOCATION_TRACE_COUNT;

    arena_allocation_trace_t &trace = arena.allocation_traces[trace_index];
    trace.file = file;
    trace.function = function;
    trace.line = line;
    trace.ptr = ptr;
    trace.size = size;
    trace.alignment = alignment;
    trace.used_after = arena.used;
    trace.allocation_index = arena.allocation_count + arena.failed_allocation_count + 1u;
    trace.error = error;
    trace.failed = failed;

    arena.allocation_trace_index = ( arena.allocation_trace_index + 1u ) % CYPHER_MEMORY_ARENA_ALLOCATION_TRACE_COUNT;
    if ( arena.allocation_trace_count < CYPHER_MEMORY_ARENA_ALLOCATION_TRACE_COUNT ) {
        ++arena.allocation_trace_count;
    }
}

/*
================
CypherMemory_ArenaDecommitToInitialCommit
================
*/
void CypherMemory_ArenaDecommitToInitialCommit( arena_t &arena )
{
    if ( arena.backing != arena_backing_t::ARENA_VIRTUAL_MEMORY ) {
        return;
    }

    if ( ( arena.flags & CYPHER_MEMORY_ARENA_FLAG_DECOMMIT_ON_RESET ) == 0u ) {
        return;
    }

    if ( arena.committed <= arena.initial_commit ) {
        return;
    }

    const common::usize decommit_offset = arena.initial_commit;
    const common::usize decommit_size = arena.committed - arena.initial_commit;
    const sys::error_code_t decommit_result = sys::CypherSystem_VirtualDecommit( arena.base + decommit_offset, decommit_size );

    if ( decommit_result != sys::error_code_t::OK ) {
        arena.last_error = error_code_t::ERR_MEMORY_DECOMMIT;
        LOG_ERROR( log::channel_t::MEMORY,
                          "arena '%s' reset failed to decommit %zu bytes.",
                          arena.name ? arena.name : "<unnamed>",
                          decommit_size );
        return;
    }

    arena.committed = arena.initial_commit;
}

}       // namespace

error_code_t CypherMemory_ArenaInit( arena_t &arena, const arena_desc_t &arena_desc )
{
    if ( arena.initialized ) {
        arena.last_error = error_code_t::ERR_ALREADY_INITIALIZED;
        LOG_WARNING( log::channel_t::MEMORY, "arena '%s' is already initialized.", arena.name ? arena.name : "<unnamed>" );
        return error_code_t::ERR_ALREADY_INITIALIZED;
    }
    if ( arena_desc.capacity == 0u ) {
        arena.last_error = error_code_t::ERR_INVALID_CAPACITY;
        LOG_ERROR( log::channel_t::MEMORY, "arena init failed for '%s': invalid capacity.", arena_desc.name ? arena_desc.name : "<unnamed>" );
        return error_code_t::ERR_INVALID_CAPACITY;
    }

    void *memory = nullptr;
    common::usize capacity = arena_desc.capacity;
    common::usize committed = arena_desc.capacity;
    common::usize page_size = 0u;
    bool owns_memory = true;

    switch ( arena_desc.backing ) {
    case arena_backing_t::ARENA_HEAP:
        memory = ::operator new( capacity, std::nothrow );
        if ( memory == nullptr ) {
            arena.last_error = error_code_t::ERR_MEMORY_ALLOCATION;
            LOG_ERROR( log::channel_t::MEMORY, "arena init failed for '%s': heap allocation of %zu bytes failed.", arena_desc.name ? arena_desc.name : "<unnamed>", capacity );
            return error_code_t::ERR_MEMORY_ALLOCATION;
        }
        break;

    case arena_backing_t::ARENA_EXTERNAL_BUFFER:
        if ( arena_desc.external_buffer == nullptr ) {
            arena.last_error = error_code_t::ERR_EXTERNAL_BUFFER_REQUIRED;
            LOG_ERROR( log::channel_t::MEMORY, "arena init failed for '%s': external buffer is required.", arena_desc.name ? arena_desc.name : "<unnamed>" );
            return error_code_t::ERR_EXTERNAL_BUFFER_REQUIRED;
        }

        memory = arena_desc.external_buffer;
        owns_memory = false;
        break;

    case arena_backing_t::ARENA_VIRTUAL_MEMORY:
        page_size = sys::CypherSystem_VirtualPageSize();

        if ( page_size == 0u || !CypherMemory_IsPowerOfTwo( page_size ) ) {
            arena.last_error = error_code_t::ERR_INVALID_ALIGNMENT;
            LOG_ERROR( log::channel_t::MEMORY, "arena init failed for '%s': invalid virtual page size %zu.", arena_desc.name ? arena_desc.name : "<unnamed>", page_size );
            return error_code_t::ERR_INVALID_ALIGNMENT;
        }

        capacity = CypherMemory_AlignSizeForward( arena_desc.capacity, page_size );

        if ( capacity == 0u ) {
            arena.last_error = error_code_t::ERR_INTEGER_OVERFLOW;
            LOG_ERROR( log::channel_t::MEMORY, "arena init failed for '%s': virtual capacity alignment overflowed.", arena_desc.name ? arena_desc.name : "<unnamed>" );
            return error_code_t::ERR_INTEGER_OVERFLOW;
        }

        committed = arena_desc.initial_commit;
        if ( committed == 0u && ( arena_desc.flags & CYPHER_MEMORY_ARENA_FLAG_GROW_COMMIT_ON_ALLOC ) == 0u ) {
            committed = capacity;
        }

        if ( committed > capacity ) {
            arena.last_error = error_code_t::ERR_INVALID_CAPACITY;
            LOG_ERROR( log::channel_t::MEMORY, "arena init failed for '%s': initial commit %zu exceeds capacity %zu.", arena_desc.name ? arena_desc.name : "<unnamed>", committed, capacity );
            return error_code_t::ERR_INVALID_CAPACITY;
        }

        committed = CypherMemory_AlignSizeForward( committed, page_size );
        if ( committed == 0u && arena_desc.initial_commit > 0u ) {
            arena.last_error = error_code_t::ERR_INTEGER_OVERFLOW;
            LOG_ERROR( log::channel_t::MEMORY, "arena init failed for '%s': initial commit alignment overflowed.", arena_desc.name ? arena_desc.name : "<unnamed>" );
            return error_code_t::ERR_INTEGER_OVERFLOW;
        }

        memory = sys::CypherSystem_VirtualReserve( capacity );

        if ( memory == nullptr ) {
            arena.last_error = error_code_t::ERR_MEMORY_RESERVE;
            LOG_ERROR( log::channel_t::MEMORY, "arena init failed for '%s': virtual reserve of %zu bytes failed.", arena_desc.name ? arena_desc.name : "<unnamed>", capacity );
            return error_code_t::ERR_MEMORY_RESERVE;
        }

        if ( committed > 0u ) {
            const sys::error_code_t commit_result = sys::CypherSystem_VirtualCommit( memory, committed );
            if ( commit_result != sys::error_code_t::OK ) {
                sys::CypherSystem_VirtualRelease( memory, capacity );
                arena.last_error = error_code_t::ERR_MEMORY_COMMIT;
                LOG_ERROR( log::channel_t::MEMORY, "arena init failed for '%s': virtual commit of %zu bytes failed.", arena_desc.name ? arena_desc.name : "<unnamed>", committed );
                return error_code_t::ERR_MEMORY_COMMIT;
            }
        }
        break;

    default:
        arena.last_error = error_code_t::ERR_INVALID_ARGUMENT;
        LOG_ERROR( log::channel_t::MEMORY, "arena init failed for '%s': invalid backing type.", arena_desc.name ? arena_desc.name : "<unnamed>" );
        return error_code_t::ERR_INVALID_ARGUMENT;
    }
    
    arena.name = arena_desc.name;
    arena.base = static_cast<common::byte *>( memory );
    
    arena.capacity = capacity;
    arena.used = 0u;
    arena.peak_used = 0u;
    
    arena.committed = committed;
    arena.initial_commit = committed;
    arena.page_size = page_size;
    
    arena.allocation_count = 0u;
    arena.failed_allocation_count = 0u;
    
    arena.flags = arena_desc.flags;
    arena.last_error = error_code_t::OK;
    
    arena.backing = arena_desc.backing;
    
    arena.initialized = true;
    arena.owns_memory = owns_memory;

    LOG_INFO( log::channel_t::MEMORY, "arena '%s' initialized: capacity=%zu bytes, committed=%zu bytes, backing=%u.", arena.name ? arena.name : "<unnamed>", arena.capacity, arena.committed, static_cast<common::u32>( arena.backing ) );
    
    return error_code_t::OK;
}

void CypherMemory_ArenaShutdown( arena_t &arena )
{
    if ( !arena.initialized ) {
        return ;
    }

    LOG_INFO( log::channel_t::MEMORY, "arena '%s' shutdown: used=%zu bytes, peak=%zu bytes, allocations=%llu, failed=%llu.",
                     arena.name ? arena.name : "<unnamed>",
                     arena.used,
                     arena.peak_used,
                     static_cast<unsigned long long>( arena.allocation_count ),
                     static_cast<unsigned long long>( arena.failed_allocation_count ) );
    
    if ( ( arena.flags & CYPHER_MEMORY_ARENA_FLAG_CLEAR_ON_SHUTDOWN ) != 0u && arena.base != nullptr && arena.used > 0u ) {
        std::memset( arena.base, 0, arena.used );
    }

    if ( arena.owns_memory && arena.base != nullptr ) {
        if ( arena.backing == arena_backing_t::ARENA_VIRTUAL_MEMORY ) {
            const sys::error_code_t release_result = sys::CypherSystem_VirtualRelease( arena.base, arena.capacity );
            if ( release_result != sys::error_code_t::OK ) {
                LOG_ERROR( log::channel_t::MEMORY, "arena '%s' virtual release failed during shutdown.", arena.name ? arena.name : "<unnamed>" );
            }
        } else {
            ::operator delete( arena.base );
        }
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
        common::usize clear_size = arena.used;
        if ( arena.backing == arena_backing_t::ARENA_VIRTUAL_MEMORY &&
             ( arena.flags & CYPHER_MEMORY_ARENA_FLAG_DECOMMIT_ON_RESET ) != 0u &&
             clear_size > arena.initial_commit ) {
            clear_size = arena.initial_commit;
        }

        if ( clear_size > 0u ) {
            std::memset( arena.base, 0, clear_size );
        }
    }

    CypherMemory_ArenaDecommitToInitialCommit( arena );
    
    arena.used = 0u;
    if ( arena.last_error != error_code_t::ERR_MEMORY_DECOMMIT ) {
        arena.last_error = error_code_t::OK;
    }
}

arena_stats_t CypherMemory_ArenaStats( const arena_t &arena )
{
    arena_stats_t stats{};
    
    stats.name = arena.name;
    stats.allocation_count = arena.allocation_count;
    stats.failed_allocation_count = arena.failed_allocation_count;
    stats.capacity = arena.capacity;
    stats.committed = arena.committed;
    stats.initial_commit = arena.initial_commit;
    stats.peak_used = arena.peak_used;
    stats.remaining = arena.used >= arena.capacity ? 0u : arena.capacity - arena.used;
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
    return CypherMemory_ArenaAllocDebug( arena, size, alignment, nullptr, nullptr, 0 );
}

void *CypherMemory_ArenaAllocDebug(
    arena_t &arena,
    common::usize size,
    common::usize alignment,
    const char *file,
    const char *function,
    common::i32 line )
{
    if ( !arena.initialized ) {
        arena.last_error = error_code_t::ERR_NOT_INITIALIZED;
        ++arena.failed_allocation_count;
        CypherMemory_ArenaRecordAllocationTrace( arena, nullptr, size, alignment, file, function, line, arena.last_error, true );
        LOG_ERROR( log::channel_t::MEMORY, "arena allocation failed: arena is not initialized." );
        return nullptr;
    }
    if ( size == 0u ) {
        arena.last_error = error_code_t::ERR_INVALID_ARGUMENT;
        ++arena.failed_allocation_count;
        CypherMemory_ArenaRecordAllocationTrace( arena, nullptr, size, alignment, file, function, line, arena.last_error, true );
        LOG_ERROR( log::channel_t::MEMORY, "arena '%s' allocation failed: requested size is zero.", arena.name ? arena.name : "<unnamed>" );
        return nullptr;
    }
    
    if ( !CypherMemory_IsPowerOfTwo( alignment ) ) {
        arena.last_error = error_code_t::ERR_INVALID_ALIGNMENT;
        ++arena.failed_allocation_count;
        CypherMemory_ArenaRecordAllocationTrace( arena, nullptr, size, alignment, file, function, line, arena.last_error, true );
        LOG_ERROR( log::channel_t::MEMORY, "arena '%s' allocation failed: invalid alignment %zu.", arena.name ? arena.name : "<unnamed>", alignment );
        return nullptr;
    } 
    
    common::usize current_address = 0u;
    if ( !CypherMemory_AddSizeChecked( reinterpret_cast<common::usize>( arena.base ), arena.used, current_address ) ) {
        arena.last_error = error_code_t::ERR_INTEGER_OVERFLOW;
        ++arena.failed_allocation_count;
        CypherMemory_ArenaRecordAllocationTrace( arena, nullptr, size, alignment, file, function, line, arena.last_error, true );
        LOG_ERROR( log::channel_t::MEMORY, "arena '%s' allocation failed: current address calculation overflowed.", arena.name ? arena.name : "<unnamed>" );
        return nullptr;
    }

    common::usize aligned_address = 0u;
    if ( !CypherMemory_AlignForwardChecked( current_address, alignment, aligned_address ) ) {
        arena.last_error = error_code_t::ERR_INTEGER_OVERFLOW;
        ++arena.failed_allocation_count;
        CypherMemory_ArenaRecordAllocationTrace( arena, nullptr, size, alignment, file, function, line, arena.last_error, true );
        LOG_ERROR( log::channel_t::MEMORY, "arena '%s' allocation failed: alignment calculation overflowed.", arena.name ? arena.name : "<unnamed>" );
        return nullptr;
    }

    const common::usize padding = aligned_address - current_address;

    common::usize used_with_padding = 0u;
    if ( !CypherMemory_AddSizeChecked( arena.used, padding, used_with_padding ) ) {
        arena.last_error = error_code_t::ERR_INTEGER_OVERFLOW;
        ++arena.failed_allocation_count;
        CypherMemory_ArenaRecordAllocationTrace( arena, nullptr, size, alignment, file, function, line, arena.last_error, true );
        LOG_ERROR( log::channel_t::MEMORY, "arena '%s' allocation failed: padding calculation overflowed.", arena.name ? arena.name : "<unnamed>" );
        return nullptr;
    }

    common::usize new_address = 0u;
    if ( !CypherMemory_AddSizeChecked( used_with_padding, size, new_address ) ) {
        arena.last_error = error_code_t::ERR_INTEGER_OVERFLOW;
        ++arena.failed_allocation_count;
        CypherMemory_ArenaRecordAllocationTrace( arena, nullptr, size, alignment, file, function, line, arena.last_error, true );
        LOG_ERROR( log::channel_t::MEMORY, "arena '%s' allocation failed: allocation end calculation overflowed.", arena.name ? arena.name : "<unnamed>" );
        return nullptr;
    }
    
    if ( new_address > arena.capacity ) {
        arena.last_error = error_code_t::ERR_OUT_OF_MEMORY;
        ++arena.failed_allocation_count;
        CypherMemory_ArenaRecordAllocationTrace( arena, nullptr, size, alignment, file, function, line, arena.last_error, true );
        LOG_ERROR( log::channel_t::MEMORY, "arena '%s' allocation failed: requested=%zu, alignment=%zu, used=%zu, capacity=%zu.", arena.name ? arena.name : "<unnamed>", size, alignment, arena.used, arena.capacity );
        return nullptr;
    }

    if ( arena.backing == arena_backing_t::ARENA_VIRTUAL_MEMORY && new_address > arena.committed ) {
        if ( ( arena.flags & CYPHER_MEMORY_ARENA_FLAG_GROW_COMMIT_ON_ALLOC ) == 0u ) {
            arena.last_error = error_code_t::ERR_MEMORY_COMMIT;
            ++arena.failed_allocation_count;
            CypherMemory_ArenaRecordAllocationTrace( arena, nullptr, size, alignment, file, function, line, arena.last_error, true );
            LOG_ERROR( log::channel_t::MEMORY, "arena '%s' allocation failed: requested=%zu exceeds committed=%zu.", arena.name ? arena.name : "<unnamed>", new_address, arena.committed );
            return nullptr;
        }

        common::usize commit_target = CypherMemory_AlignSizeForward( new_address, arena.page_size );
        if ( commit_target == 0u ) {
            arena.last_error = error_code_t::ERR_INTEGER_OVERFLOW;
            ++arena.failed_allocation_count;
            CypherMemory_ArenaRecordAllocationTrace( arena, nullptr, size, alignment, file, function, line, arena.last_error, true );
            LOG_ERROR( log::channel_t::MEMORY, "arena '%s' allocation failed: commit target calculation overflowed.", arena.name ? arena.name : "<unnamed>" );
            return nullptr;
        }

        if ( commit_target > arena.capacity ) {
            commit_target = arena.capacity;
        }

        const common::usize commit_size = commit_target - arena.committed;
        const sys::error_code_t commit_result = sys::CypherSystem_VirtualCommit( arena.base + arena.committed, commit_size );

        if ( commit_result != sys::error_code_t::OK ) {
            arena.last_error = error_code_t::ERR_MEMORY_COMMIT;
            ++arena.failed_allocation_count;
            CypherMemory_ArenaRecordAllocationTrace( arena, nullptr, size, alignment, file, function, line, arena.last_error, true );
            LOG_ERROR( log::channel_t::MEMORY, "arena '%s' allocation failed: virtual commit of %zu bytes failed.", arena.name ? arena.name : "<unnamed>", commit_size );
            return nullptr;
        }

        arena.committed = commit_target;
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

    CypherMemory_ArenaRecordAllocationTrace( arena, result, size, alignment, file, function, line, arena.last_error, false );
    
    return result;
}

void *CypherMemory_ArenaAllocZero( arena_t &arena, common::usize size, common::usize alignment )
{
    return CypherMemory_ArenaAllocZeroDebug( arena, size, alignment, nullptr, nullptr, 0 );
}

void *CypherMemory_ArenaAllocZeroDebug(
    arena_t &arena,
    common::usize size,
    common::usize alignment,
    const char *file,
    const char *function,
    common::i32 line )
{
    void *memory = CypherMemory_ArenaAllocDebug( arena, size, alignment, file, function, line );
    
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

const arena_allocation_trace_t *CypherMemory_ArenaAllocationTraces( const arena_t &arena, common::usize &out_count )
{
    out_count = arena.allocation_trace_count;
    return arena.allocation_traces;
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
        LOG_ERROR( log::channel_t::MEMORY, "arena rewind failed: arena is not initialized." );
        return error_code_t::ERR_NOT_INITIALIZED;
    }
    
    if ( marker.used > arena.used || marker.used > arena.capacity ) {
        arena.last_error = error_code_t::ERR_INVALID_MARKER;
        LOG_ERROR( log::channel_t::MEMORY, "arena '%s' rewind failed: marker=%zu, used=%zu, capacity=%zu.", arena.name ? arena.name : "<unnamed>", marker.used, arena.used, arena.capacity );
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
    const common::usize base = reinterpret_cast<common::usize>( arena.base );
    
    if ( address < base ) {
        return false;
    }

    return ( address - base ) < arena.capacity;
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
    if ( !arena.initialized ) {
        return 0u;
    }
    
    return arena.capacity;
}

}       // namespace cypher::engine::memory

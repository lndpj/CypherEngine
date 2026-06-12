/*======================================================================
   File: CypherMemory_Pool.cpp
   Project: CYPHER
   Author: ksiric <email@example.com>
   Created: 2026-06-12
   ---------------------------------------------------------------------
   Description:
       Fixed-block pool allocator implementation.
   ---------------------------------------------------------------------
   License:
   Company:
   Version: 0.1.0
 ======================================================================
                                                                       */

#include "CypherEngine/CypherMemory/CypherMemory_Pool.h"
#include "CypherEngine/CypherLog/CypherLog.h"

#include <cstring>
#include <limits>

namespace cypher::engine::memory
{

struct pool_free_node_t {
    pool_free_node_t *next{ nullptr };
};

namespace {

constexpr common::usize CYPHER_MEMORY_POOL_INVALID_SLOT_INDEX = std::numeric_limits<common::usize>::max();
constexpr common::usize CYPHER_MEMORY_POOL_ALLOCATION_BITS_PER_WORD = sizeof( common::u64 ) * 8u;

struct pool_layout_t {
    common::usize slot_size{ 0u };
    common::usize slot_stride{ 0u };
    common::usize slot_count{ 0u };
    common::usize alignment{ CYPHER_MEMORY_DEFAULT_ALIGNMENT };
    common::usize slot_bytes{ 0u };
    common::usize metadata_offset{ 0u };
    common::usize metadata_bytes{ 0u };
    common::usize backing_bytes{ 0u };
    common::usize allocation_word_count{ 0u };
};

common::usize CypherMemory_PoolMax( const common::usize a, const common::usize b )
{
    return a > b ? a : b;
}

bool CypherMemory_PoolAddChecked( const common::usize a, const common::usize b, common::usize &out_value )
{
    const common::usize max_value = std::numeric_limits<common::usize>::max();

    if ( a > max_value - b ) {
        return false;
    }

    out_value = a + b;
    return true;
}

bool CypherMemory_PoolMulChecked( const common::usize a, const common::usize b, common::usize &out_value )
{
    const common::usize max_value = std::numeric_limits<common::usize>::max();

    if ( a != 0u && b > max_value / a ) {
        return false;
    }

    out_value = a * b;
    return true;
}

bool CypherMemory_PoolAlignForwardChecked( const common::usize value,
                                           const common::usize alignment,
                                           common::usize &out_value )
{
    if ( !CypherMemory_IsPowerOfTwo( alignment ) ) {
        return false;
    }

    const common::usize mask = alignment - 1u;
    const common::usize max_value = std::numeric_limits<common::usize>::max();

    if ( value > max_value - mask ) {
        return false;
    }

    out_value = ( value + mask ) & ~mask;
    return true;
}

error_code_t CypherMemory_PoolFailInit( pool_t &pool, const pool_desc_t &pool_desc, const error_code_t error, const char *reason )
{
    pool.name = pool_desc.name;
    pool.last_error = error;

    LOG_ERROR( log::channel_t::MEMORY,
                      "pool '%s' init failed: %s.",
                      pool.name ? pool.name : "<unnamed>",
                      reason ? reason : CypherMemory_ErrorDesc( error ) );

    return error;
}

bool CypherMemory_PoolComputeLayout( const pool_desc_t &pool_desc, pool_layout_t &out_layout, error_code_t &out_error )
{
    out_layout = {};
    out_error = error_code_t::OK;

    if ( pool_desc.slot_size == 0u ) {
        out_error = error_code_t::ERR_INVALID_ARGUMENT;
        return false;
    }

    if ( pool_desc.slot_count == 0u ) {
        out_error = error_code_t::ERR_INVALID_CAPACITY;
        return false;
    }

    if ( !CypherMemory_IsPowerOfTwo( pool_desc.alignment ) ) {
        out_error = error_code_t::ERR_INVALID_ALIGNMENT;
        return false;
    }

    const common::usize node_alignment = alignof( pool_free_node_t );
    const common::usize node_size = sizeof( pool_free_node_t );
    const common::usize alignment = CypherMemory_PoolMax( pool_desc.alignment, node_alignment );
    const common::usize slot_payload_size = CypherMemory_PoolMax( pool_desc.slot_size, node_size );

    common::usize slot_stride = 0u;
    if ( !CypherMemory_PoolAlignForwardChecked( slot_payload_size, alignment, slot_stride ) ) {
        out_error = error_code_t::ERR_INTEGER_OVERFLOW;
        return false;
    }

    common::usize slot_bytes = 0u;
    if ( !CypherMemory_PoolMulChecked( slot_stride, pool_desc.slot_count, slot_bytes ) ) {
        out_error = error_code_t::ERR_INTEGER_OVERFLOW;
        return false;
    }

    const common::usize bits_rounding = CYPHER_MEMORY_POOL_ALLOCATION_BITS_PER_WORD - 1u;
    common::usize rounded_slot_count = 0u;
    if ( !CypherMemory_PoolAddChecked( pool_desc.slot_count, bits_rounding, rounded_slot_count ) ) {
        out_error = error_code_t::ERR_INTEGER_OVERFLOW;
        return false;
    }

    const common::usize allocation_word_count = rounded_slot_count / CYPHER_MEMORY_POOL_ALLOCATION_BITS_PER_WORD;

    common::usize metadata_bytes = 0u;
    if ( !CypherMemory_PoolMulChecked( allocation_word_count, sizeof( common::u64 ), metadata_bytes ) ) {
        out_error = error_code_t::ERR_INTEGER_OVERFLOW;
        return false;
    }

    common::usize metadata_offset = 0u;
    if ( !CypherMemory_PoolAlignForwardChecked( slot_bytes, alignof( common::u64 ), metadata_offset ) ) {
        out_error = error_code_t::ERR_INTEGER_OVERFLOW;
        return false;
    }

    common::usize backing_bytes = 0u;
    if ( !CypherMemory_PoolAddChecked( metadata_offset, metadata_bytes, backing_bytes ) ) {
        out_error = error_code_t::ERR_INTEGER_OVERFLOW;
        return false;
    }

    out_layout.slot_size = pool_desc.slot_size;
    out_layout.slot_stride = slot_stride;
    out_layout.slot_count = pool_desc.slot_count;
    out_layout.alignment = alignment;
    out_layout.slot_bytes = slot_bytes;
    out_layout.metadata_offset = metadata_offset;
    out_layout.metadata_bytes = metadata_bytes;
    out_layout.backing_bytes = backing_bytes;
    out_layout.allocation_word_count = allocation_word_count;

    return true;
}

void CypherMemory_PoolRecordOperationTrace( pool_t &pool,
                                            void *ptr,
                                            const common::usize slot_index,
                                            const pool_operation_t operation,
                                            const error_code_t error,
                                            const bool failed,
                                            const char *file,
                                            const char *function,
                                            const common::i32 line )
{
    const common::usize trace_index = pool.operation_trace_index % CYPHER_MEMORY_POOL_OPERATION_TRACE_COUNT;

    pool_operation_trace_t &trace = pool.operation_traces[trace_index];
    trace.file = file;
    trace.function = function;
    trace.line = line;
    trace.ptr = ptr;
    trace.slot_index = slot_index;
    trace.operation = operation;
    trace.operation_index = pool.allocation_count +
                            pool.free_operation_count +
                            pool.failed_allocation_count +
                            pool.failed_free_count;
    trace.error = error;
    trace.failed = failed;

    pool.operation_trace_index = ( pool.operation_trace_index + 1u ) % CYPHER_MEMORY_POOL_OPERATION_TRACE_COUNT;
    if ( pool.operation_trace_count < CYPHER_MEMORY_POOL_OPERATION_TRACE_COUNT ) {
        ++pool.operation_trace_count;
    }
}

common::byte *CypherMemory_PoolSlotPtr( const pool_t &pool, const common::usize slot_index )
{
    return pool.base + ( slot_index * pool.slot_stride );
}

common::usize CypherMemory_PoolIndexUnchecked( const pool_t &pool, const void *ptr )
{
    const common::usize address = reinterpret_cast<common::usize>( ptr );
    const common::usize base = reinterpret_cast<common::usize>( pool.base );
    return ( address - base ) / pool.slot_stride;
}

bool CypherMemory_PoolIsSlotAllocated( const pool_t &pool, const common::usize slot_index )
{
    const common::usize word_index = slot_index / CYPHER_MEMORY_POOL_ALLOCATION_BITS_PER_WORD;
    const common::usize bit_index = slot_index % CYPHER_MEMORY_POOL_ALLOCATION_BITS_PER_WORD;
    const common::u64 mask = common::u64{ 1u } << bit_index;

    return ( pool.allocation_bits[word_index] & mask ) != 0u;
}

void CypherMemory_PoolSetSlotAllocated( pool_t &pool, const common::usize slot_index )
{
    const common::usize word_index = slot_index / CYPHER_MEMORY_POOL_ALLOCATION_BITS_PER_WORD;
    const common::usize bit_index = slot_index % CYPHER_MEMORY_POOL_ALLOCATION_BITS_PER_WORD;
    const common::u64 mask = common::u64{ 1u } << bit_index;

    pool.allocation_bits[word_index] |= mask;
}

void CypherMemory_PoolClearSlotAllocated( pool_t &pool, const common::usize slot_index )
{
    const common::usize word_index = slot_index / CYPHER_MEMORY_POOL_ALLOCATION_BITS_PER_WORD;
    const common::usize bit_index = slot_index % CYPHER_MEMORY_POOL_ALLOCATION_BITS_PER_WORD;
    const common::u64 mask = common::u64{ 1u } << bit_index;

    pool.allocation_bits[word_index] &= ~mask;
}

void CypherMemory_PoolClearAllocationBits( pool_t &pool )
{
    if ( pool.allocation_bits == nullptr || pool.metadata_bytes == 0u ) {
        return;
    }

    std::memset( pool.allocation_bits, 0, pool.metadata_bytes );
}

void CypherMemory_PoolBuildFreeList( pool_t &pool )
{
    pool.free_list = nullptr;

    for ( common::usize slot_index = pool.slot_count; slot_index > 0u; --slot_index ) {
        pool_free_node_t *node = reinterpret_cast<pool_free_node_t *>( CypherMemory_PoolSlotPtr( pool, slot_index - 1u ) );
        node->next = pool.free_list;
        pool.free_list = node;
    }

    pool.used_count = 0u;
    pool.free_count = pool.slot_count;
}

void *CypherMemory_PoolAllocInternal( pool_t &pool,
                                      const common::usize requested_size,
                                      const common::usize requested_alignment,
                                      const bool zero_memory,
                                      const char *file,
                                      const char *function,
                                      const common::i32 line )
{
    if ( !pool.initialized ) {
        pool.last_error = error_code_t::ERR_NOT_INITIALIZED;
        ++pool.failed_allocation_count;
        CypherMemory_PoolRecordOperationTrace( pool, nullptr, CYPHER_MEMORY_POOL_INVALID_SLOT_INDEX, pool_operation_t::POOL_OPERATION_ALLOC, pool.last_error, true, file, function, line );
        LOG_ERROR( log::channel_t::MEMORY, "pool allocation failed: pool is not initialized." );
        return nullptr;
    }

    if ( requested_size == 0u ) {
        pool.last_error = error_code_t::ERR_INVALID_ARGUMENT;
        ++pool.failed_allocation_count;
        CypherMemory_PoolRecordOperationTrace( pool, nullptr, CYPHER_MEMORY_POOL_INVALID_SLOT_INDEX, pool_operation_t::POOL_OPERATION_ALLOC, pool.last_error, true, file, function, line );
        LOG_ERROR( log::channel_t::MEMORY, "pool '%s' allocation failed: requested size is zero.", pool.name ? pool.name : "<unnamed>" );
        return nullptr;
    }

    if ( !CypherMemory_IsPowerOfTwo( requested_alignment ) ) {
        pool.last_error = error_code_t::ERR_INVALID_ALIGNMENT;
        ++pool.failed_allocation_count;
        CypherMemory_PoolRecordOperationTrace( pool, nullptr, CYPHER_MEMORY_POOL_INVALID_SLOT_INDEX, pool_operation_t::POOL_OPERATION_ALLOC, pool.last_error, true, file, function, line );
        LOG_ERROR( log::channel_t::MEMORY, "pool '%s' allocation failed: invalid alignment %zu.", pool.name ? pool.name : "<unnamed>", requested_alignment );
        return nullptr;
    }

    if ( requested_size > pool.slot_size ) {
        pool.last_error = error_code_t::ERR_BUFFER_TOO_SMALL;
        ++pool.failed_allocation_count;
        CypherMemory_PoolRecordOperationTrace( pool, nullptr, CYPHER_MEMORY_POOL_INVALID_SLOT_INDEX, pool_operation_t::POOL_OPERATION_ALLOC, pool.last_error, true, file, function, line );
        LOG_ERROR( log::channel_t::MEMORY,
                          "pool '%s' allocation failed: requested=%zu, slot_size=%zu.",
                          pool.name ? pool.name : "<unnamed>",
                          requested_size,
                          pool.slot_size );
        return nullptr;
    }

    if ( requested_alignment > pool.alignment ) {
        pool.last_error = error_code_t::ERR_INVALID_ALIGNMENT;
        ++pool.failed_allocation_count;
        CypherMemory_PoolRecordOperationTrace( pool, nullptr, CYPHER_MEMORY_POOL_INVALID_SLOT_INDEX, pool_operation_t::POOL_OPERATION_ALLOC, pool.last_error, true, file, function, line );
        LOG_ERROR( log::channel_t::MEMORY,
                          "pool '%s' allocation failed: requested alignment=%zu exceeds pool alignment=%zu.",
                          pool.name ? pool.name : "<unnamed>",
                          requested_alignment,
                          pool.alignment );
        return nullptr;
    }

    if ( pool.free_list == nullptr || pool.free_count == 0u ) {
        pool.last_error = error_code_t::ERR_OUT_OF_MEMORY;
        ++pool.failed_allocation_count;
        CypherMemory_PoolRecordOperationTrace( pool, nullptr, CYPHER_MEMORY_POOL_INVALID_SLOT_INDEX, pool_operation_t::POOL_OPERATION_ALLOC, pool.last_error, true, file, function, line );
        LOG_ERROR( log::channel_t::MEMORY, "pool '%s' allocation failed: pool is full.", pool.name ? pool.name : "<unnamed>" );
        return nullptr;
    }

    pool_free_node_t *node = pool.free_list;
    pool.free_list = node->next;

    void *result = static_cast<void *>( node );
    const common::usize slot_index = CypherMemory_PoolIndexUnchecked( pool, result );

    CypherMemory_PoolSetSlotAllocated( pool, slot_index );

    ++pool.used_count;
    --pool.free_count;

    if ( pool.used_count > pool.peak_used_count ) {
        pool.peak_used_count = pool.used_count;
    }

    ++pool.allocation_count;
    pool.last_error = error_code_t::OK;

    if ( zero_memory || ( pool.flags & CYPHER_MEMORY_POOL_FLAG_ZERO_ON_ALLOC ) != 0u ) {
        std::memset( result, 0, pool.slot_stride );
    }

    CypherMemory_PoolRecordOperationTrace( pool, result, slot_index, pool_operation_t::POOL_OPERATION_ALLOC, pool.last_error, false, file, function, line );

    return result;
}

}       // namespace

error_code_t CypherMemory_PoolInit( pool_t &pool, const pool_desc_t &pool_desc )
{
    if ( pool.initialized ) {
        pool.last_error = error_code_t::ERR_ALREADY_INITIALIZED;
        LOG_WARNING( log::channel_t::MEMORY, "pool '%s' is already initialized.", pool.name ? pool.name : "<unnamed>" );
        return error_code_t::ERR_ALREADY_INITIALIZED;
    }

    pool_layout_t layout{};
    error_code_t layout_error = error_code_t::OK;
    if ( !CypherMemory_PoolComputeLayout( pool_desc, layout, layout_error ) ) {
        return CypherMemory_PoolFailInit( pool, pool_desc, layout_error, CypherMemory_ErrorDesc( layout_error ) );
    }

    void *memory = nullptr;

    switch ( pool_desc.backing ) {
    case pool_backing_t::POOL_ARENA:
        if ( pool_desc.arena == nullptr ) {
            return CypherMemory_PoolFailInit( pool, pool_desc, error_code_t::ERR_INVALID_ARGUMENT, "arena pointer is required" );
        }

        if ( !CypherMemory_ArenaIsInitialized( *pool_desc.arena ) ) {
            return CypherMemory_PoolFailInit( pool, pool_desc, error_code_t::ERR_NOT_INITIALIZED, "backing arena is not initialized" );
        }

        memory = CypherMemory_ArenaAlloc( *pool_desc.arena, layout.backing_bytes, layout.alignment );
        if ( memory == nullptr ) {
            error_code_t arena_error = CypherMemory_ArenaLastError( *pool_desc.arena );
            if ( arena_error == error_code_t::OK ) {
                arena_error = error_code_t::ERR_OUT_OF_MEMORY;
            }
            return CypherMemory_PoolFailInit( pool, pool_desc, arena_error, CypherMemory_ErrorDesc( arena_error ) );
        }
        break;

    case pool_backing_t::POOL_EXTERNAL_BUFFER:
        if ( pool_desc.external_buffer == nullptr ) {
            return CypherMemory_PoolFailInit( pool, pool_desc, error_code_t::ERR_EXTERNAL_BUFFER_REQUIRED, "external buffer is required" );
        }

        if ( pool_desc.external_buffer_size == 0u ) {
            return CypherMemory_PoolFailInit( pool, pool_desc, error_code_t::ERR_BUFFER_TOO_SMALL, "external buffer size is zero" );
        }

        {
            const common::usize external_address = reinterpret_cast<common::usize>( pool_desc.external_buffer );
            common::usize aligned_address = 0u;

            if ( !CypherMemory_PoolAlignForwardChecked( external_address, layout.alignment, aligned_address ) ) {
                return CypherMemory_PoolFailInit( pool, pool_desc, error_code_t::ERR_INTEGER_OVERFLOW, "external buffer alignment overflow" );
            }

            const common::usize padding = aligned_address - external_address;
            if ( padding > pool_desc.external_buffer_size ) {
                return CypherMemory_PoolFailInit( pool, pool_desc, error_code_t::ERR_BUFFER_TOO_SMALL, "external buffer is too small after alignment padding" );
            }

            const common::usize available_size = pool_desc.external_buffer_size - padding;
            if ( available_size < layout.backing_bytes ) {
                return CypherMemory_PoolFailInit( pool, pool_desc, error_code_t::ERR_BUFFER_TOO_SMALL, "external buffer cannot fit pool slots and metadata" );
            }

            memory = reinterpret_cast<void *>( aligned_address );
        }
        break;

    default:
        return CypherMemory_PoolFailInit( pool, pool_desc, error_code_t::ERR_INVALID_ARGUMENT, "invalid backing type" );
    }

    pool = {};
    pool.name = pool_desc.name;
    pool.base = static_cast<common::byte *>( memory );
    pool.free_list = nullptr;
    pool.allocation_bits = reinterpret_cast<common::u64 *>( pool.base + layout.metadata_offset );

    pool.slot_size = layout.slot_size;
    pool.slot_stride = layout.slot_stride;
    pool.slot_count = layout.slot_count;
    pool.alignment = layout.alignment;

    pool.slot_bytes = layout.slot_bytes;
    pool.metadata_bytes = layout.metadata_bytes;
    pool.backing_bytes = layout.backing_bytes;
    pool.allocation_word_count = layout.allocation_word_count;

    pool.used_count = 0u;
    pool.free_count = layout.slot_count;
    pool.peak_used_count = 0u;

    pool.flags = pool_desc.flags;
    pool.backing = pool_desc.backing;
    pool.last_error = error_code_t::OK;
    pool.initialized = true;

    CypherMemory_PoolClearAllocationBits( pool );
    CypherMemory_PoolBuildFreeList( pool );

    LOG_INFO( log::channel_t::MEMORY,
                     "pool '%s' initialized: slot_size=%zu, slot_stride=%zu, slot_count=%zu, backing_bytes=%zu.",
                     pool.name ? pool.name : "<unnamed>",
                     pool.slot_size,
                     pool.slot_stride,
                     pool.slot_count,
                     pool.backing_bytes );

    return error_code_t::OK;
}

void CypherMemory_PoolShutdown( pool_t &pool )
{
    if ( !pool.initialized ) {
        return;
    }

    LOG_INFO( log::channel_t::MEMORY,
                     "pool '%s' shutdown: used=%zu, peak=%zu, allocations=%llu, frees=%llu, failed_alloc=%llu, failed_free=%llu.",
                     pool.name ? pool.name : "<unnamed>",
                     pool.used_count,
                     pool.peak_used_count,
                     static_cast<unsigned long long>( pool.allocation_count ),
                     static_cast<unsigned long long>( pool.free_operation_count ),
                     static_cast<unsigned long long>( pool.failed_allocation_count ),
                     static_cast<unsigned long long>( pool.failed_free_count ) );

    if ( ( pool.flags & CYPHER_MEMORY_POOL_FLAG_CLEAR_ON_SHUTDOWN ) != 0u && pool.base != nullptr && pool.backing_bytes > 0u ) {
        std::memset( pool.base, 0, pool.backing_bytes );
    }

    pool = {};
}

pool_stats_t CypherMemory_PoolStats( const pool_t &pool )
{
    pool_stats_t stats{};

    stats.name = pool.name;
    stats.slot_size = pool.slot_size;
    stats.slot_stride = pool.slot_stride;
    stats.slot_count = pool.slot_count;
    stats.used_count = pool.used_count;
    stats.free_count = pool.free_count;
    stats.peak_used_count = pool.peak_used_count;
    stats.slot_bytes = pool.slot_bytes;
    stats.metadata_bytes = pool.metadata_bytes;
    stats.backing_bytes = pool.backing_bytes;
    stats.allocation_count = pool.allocation_count;
    stats.free_operation_count = pool.free_operation_count;
    stats.failed_allocation_count = pool.failed_allocation_count;
    stats.failed_free_count = pool.failed_free_count;

    return stats;
}

void CypherMemory_PoolResetCounters( pool_t &pool )
{
    if ( !pool.initialized ) {
        return;
    }

    pool.peak_used_count = pool.used_count;
    pool.allocation_count = 0u;
    pool.free_operation_count = 0u;
    pool.failed_allocation_count = 0u;
    pool.failed_free_count = 0u;
    pool.operation_trace_index = 0u;
    pool.operation_trace_count = 0u;
    pool.last_error = error_code_t::OK;
}

void CypherMemory_PoolReset( pool_t &pool )
{
    if ( !pool.initialized ) {
        return;
    }

    if ( ( pool.flags & CYPHER_MEMORY_POOL_FLAG_CLEAR_ON_RESET ) != 0u && pool.base != nullptr && pool.slot_bytes > 0u ) {
        std::memset( pool.base, 0, pool.slot_bytes );
    }

    CypherMemory_PoolClearAllocationBits( pool );
    CypherMemory_PoolBuildFreeList( pool );

    pool.last_error = error_code_t::OK;
    CypherMemory_PoolRecordOperationTrace( pool, nullptr, CYPHER_MEMORY_POOL_INVALID_SLOT_INDEX, pool_operation_t::POOL_OPERATION_RESET, pool.last_error, false, nullptr, nullptr, 0 );
}

void *CypherMemory_PoolAlloc( pool_t &pool )
{
    return CypherMemory_PoolAllocDebug( pool, nullptr, nullptr, 0 );
}

void *CypherMemory_PoolAllocDebug( pool_t &pool, const char *file, const char *function, common::i32 line )
{
    return CypherMemory_PoolAllocInternal( pool, pool.slot_size, pool.alignment, false, file, function, line );
}

void *CypherMemory_PoolAllocZero( pool_t &pool )
{
    return CypherMemory_PoolAllocZeroDebug( pool, nullptr, nullptr, 0 );
}

void *CypherMemory_PoolAllocZeroDebug( pool_t &pool, const char *file, const char *function, common::i32 line )
{
    return CypherMemory_PoolAllocInternal( pool, pool.slot_size, pool.alignment, true, file, function, line );
}

void *CypherMemory_PoolAllocSize( pool_t &pool, common::usize size, common::usize alignment )
{
    return CypherMemory_PoolAllocSizeDebug( pool, size, alignment, nullptr, nullptr, 0 );
}

void *CypherMemory_PoolAllocSizeDebug( pool_t &pool,
                                       common::usize size,
                                       common::usize alignment,
                                       const char *file,
                                       const char *function,
                                       common::i32 line )
{
    return CypherMemory_PoolAllocInternal( pool, size, alignment, false, file, function, line );
}

void *CypherMemory_PoolAllocSizeZero( pool_t &pool, common::usize size, common::usize alignment )
{
    return CypherMemory_PoolAllocSizeZeroDebug( pool, size, alignment, nullptr, nullptr, 0 );
}

void *CypherMemory_PoolAllocSizeZeroDebug( pool_t &pool,
                                           common::usize size,
                                           common::usize alignment,
                                           const char *file,
                                           const char *function,
                                           common::i32 line )
{
    return CypherMemory_PoolAllocInternal( pool, size, alignment, true, file, function, line );
}

error_code_t CypherMemory_PoolFree( pool_t &pool, void *ptr )
{
    return CypherMemory_PoolFreeDebug( pool, ptr, nullptr, nullptr, 0 );
}

error_code_t CypherMemory_PoolFreeDebug( pool_t &pool, void *ptr, const char *file, const char *function, common::i32 line )
{
    if ( !pool.initialized ) {
        pool.last_error = error_code_t::ERR_NOT_INITIALIZED;
        ++pool.failed_free_count;
        CypherMemory_PoolRecordOperationTrace( pool, ptr, CYPHER_MEMORY_POOL_INVALID_SLOT_INDEX, pool_operation_t::POOL_OPERATION_FREE, pool.last_error, true, file, function, line );
        LOG_ERROR( log::channel_t::MEMORY, "pool free failed: pool is not initialized." );
        return pool.last_error;
    }

    if ( ptr == nullptr ) {
        pool.last_error = error_code_t::ERR_INVALID_POINTER;
        ++pool.failed_free_count;
        CypherMemory_PoolRecordOperationTrace( pool, ptr, CYPHER_MEMORY_POOL_INVALID_SLOT_INDEX, pool_operation_t::POOL_OPERATION_FREE, pool.last_error, true, file, function, line );
        LOG_ERROR( log::channel_t::MEMORY, "pool '%s' free failed: pointer is null.", pool.name ? pool.name : "<unnamed>" );
        return pool.last_error;
    }

    if ( !CypherMemory_PoolOwnsSlot( pool, ptr ) ) {
        pool.last_error = error_code_t::ERR_INVALID_POINTER;
        ++pool.failed_free_count;
        CypherMemory_PoolRecordOperationTrace( pool, ptr, CYPHER_MEMORY_POOL_INVALID_SLOT_INDEX, pool_operation_t::POOL_OPERATION_FREE, pool.last_error, true, file, function, line );
        LOG_ERROR( log::channel_t::MEMORY, "pool '%s' free failed: pointer does not belong to a pool slot.", pool.name ? pool.name : "<unnamed>" );
        return pool.last_error;
    }

    const common::usize slot_index = CypherMemory_PoolIndexUnchecked( pool, ptr );

    if ( !CypherMemory_PoolIsSlotAllocated( pool, slot_index ) ) {
        pool.last_error = error_code_t::ERR_DOUBLE_FREE;
        ++pool.failed_free_count;
        CypherMemory_PoolRecordOperationTrace( pool, ptr, slot_index, pool_operation_t::POOL_OPERATION_FREE, pool.last_error, true, file, function, line );
        LOG_ERROR( log::channel_t::MEMORY, "pool '%s' free failed: slot %zu is already free.", pool.name ? pool.name : "<unnamed>", slot_index );
        return pool.last_error;
    }

    if ( ( pool.flags & CYPHER_MEMORY_POOL_FLAG_CLEAR_ON_FREE ) != 0u ) {
        std::memset( ptr, 0, pool.slot_stride );
    }

    pool_free_node_t *node = static_cast<pool_free_node_t *>( ptr );
    node->next = pool.free_list;
    pool.free_list = node;

    CypherMemory_PoolClearSlotAllocated( pool, slot_index );

    --pool.used_count;
    ++pool.free_count;
    ++pool.free_operation_count;
    pool.last_error = error_code_t::OK;

    CypherMemory_PoolRecordOperationTrace( pool, ptr, slot_index, pool_operation_t::POOL_OPERATION_FREE, pool.last_error, false, file, function, line );

    return pool.last_error;
}

bool CypherMemory_PoolContains( const pool_t &pool, const void *ptr )
{
    if ( !pool.initialized || pool.base == nullptr || ptr == nullptr ) {
        return false;
    }

    const common::usize address = reinterpret_cast<common::usize>( ptr );
    const common::usize base = reinterpret_cast<common::usize>( pool.base );

    if ( address < base ) {
        return false;
    }

    return ( address - base ) < pool.slot_bytes;
}

bool CypherMemory_PoolOwnsSlot( const pool_t &pool, const void *ptr )
{
    if ( !CypherMemory_PoolContains( pool, ptr ) || pool.slot_stride == 0u ) {
        return false;
    }

    const common::usize address = reinterpret_cast<common::usize>( ptr );
    const common::usize base = reinterpret_cast<common::usize>( pool.base );
    const common::usize offset = address - base;

    return ( offset % pool.slot_stride ) == 0u;
}

bool CypherMemory_PoolIsInitialized( const pool_t &pool )
{
    return pool.initialized;
}

error_code_t CypherMemory_PoolLastError( const pool_t &pool )
{
    return pool.last_error;
}

common::usize CypherMemory_PoolSlotIndex( const pool_t &pool, const void *ptr )
{
    if ( !CypherMemory_PoolOwnsSlot( pool, ptr ) ) {
        return CYPHER_MEMORY_POOL_INVALID_SLOT_INDEX;
    }

    return CypherMemory_PoolIndexUnchecked( pool, ptr );
}

common::usize CypherMemory_PoolUsedCount( const pool_t &pool )
{
    return pool.used_count;
}

common::usize CypherMemory_PoolFreeCount( const pool_t &pool )
{
    return pool.free_count;
}

common::usize CypherMemory_PoolCapacity( const pool_t &pool )
{
    return pool.slot_count;
}

common::f32 CypherMemory_PoolUsageRatio( const pool_t &pool )
{
    if ( pool.slot_count == 0u ) {
        return 0.0f;
    }

    return static_cast<common::f32>( pool.used_count ) / static_cast<common::f32>( pool.slot_count );
}

const pool_operation_trace_t *CypherMemory_PoolOperationTraces( const pool_t &pool, common::usize &out_count )
{
    out_count = pool.operation_trace_count;
    return pool.operation_traces;
}

}       // namespace cypher::engine::memory

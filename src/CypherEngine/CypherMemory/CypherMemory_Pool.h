#pragma once

#include "CypherEngine/CypherCommon/CypherCommon.h"
#include "CypherEngine/CypherMemory/CypherMemory_Arena.h"

namespace cypher::engine::memory
{

/*
================
Pool Constants
================
*/
constexpr common::u32 CYPHER_MEMORY_POOL_FLAG_NONE              = 0u;
constexpr common::u32 CYPHER_MEMORY_POOL_FLAG_ZERO_ON_ALLOC     = 1u << 0u;
constexpr common::u32 CYPHER_MEMORY_POOL_FLAG_CLEAR_ON_FREE     = 1u << 1u;
constexpr common::u32 CYPHER_MEMORY_POOL_FLAG_CLEAR_ON_RESET    = 1u << 2u;
constexpr common::u32 CYPHER_MEMORY_POOL_FLAG_CLEAR_ON_SHUTDOWN = 1u << 3u;

constexpr common::usize CYPHER_MEMORY_POOL_OPERATION_TRACE_COUNT = 64u;

enum class pool_backing_t : common::u8 {
    POOL_ARENA = 0,
    POOL_EXTERNAL_BUFFER
};

enum class pool_operation_t : common::u8 {
    POOL_OPERATION_ALLOC = 0,
    POOL_OPERATION_FREE,
    POOL_OPERATION_RESET
};

/*
================
Pool Description

Creation request for a fixed-block allocator. A pool never owns an arena or
external buffer; it only manages slots inside memory provided by them.
================
*/
struct pool_desc_t {
    const char *name{ nullptr };

    arena_t *arena{ nullptr };
    void *external_buffer{ nullptr };
    common::usize external_buffer_size{ 0u };

    common::usize slot_size{ 0u };
    common::usize slot_count{ 0u };
    common::usize alignment{ CYPHER_MEMORY_DEFAULT_ALIGNMENT };

    common::u32 flags{ CYPHER_MEMORY_POOL_FLAG_NONE };
    pool_backing_t backing{ pool_backing_t::POOL_ARENA };
};

struct pool_free_node_t;

/*
================
Pool Operation Trace

Small ring buffer for recent pool operations. This is diagnostic data only.
================
*/
struct pool_operation_trace_t {
    const char *file{ nullptr };
    const char *function{ nullptr };
    common::i32 line{ 0 };

    void *ptr{ nullptr };
    common::usize slot_index{ 0u };

    common::u64 operation_index{ 0u };
    pool_operation_t operation{ pool_operation_t::POOL_OPERATION_ALLOC };
    error_code_t error{ error_code_t::OK };
    bool failed{ false };
};

/*
================
Pool Stats

Snapshot of fixed-block pool usage.
================
*/
struct pool_stats_t {
    const char *name{ nullptr };

    common::usize slot_size{ 0u };
    common::usize slot_stride{ 0u };
    common::usize slot_count{ 0u };
    common::usize used_count{ 0u };
    common::usize free_count{ 0u };
    common::usize peak_used_count{ 0u };

    common::usize slot_bytes{ 0u };
    common::usize metadata_bytes{ 0u };
    common::usize backing_bytes{ 0u };

    common::u64 allocation_count{ 0u };
    common::u64 free_operation_count{ 0u };
    common::u64 failed_allocation_count{ 0u };
    common::u64 failed_free_count{ 0u };
};

/*
================
Pool

Fixed-block allocator state. The free list is stored inside free slots, while
the allocation bitmap detects invalid frees and double frees.
================
*/
struct pool_t {
    const char *name{ nullptr };

    common::byte *base{ nullptr };
    pool_free_node_t *free_list{ nullptr };
    common::u64 *allocation_bits{ nullptr };

    common::usize slot_size{ 0u };
    common::usize slot_stride{ 0u };
    common::usize slot_count{ 0u };
    common::usize alignment{ CYPHER_MEMORY_DEFAULT_ALIGNMENT };

    common::usize slot_bytes{ 0u };
    common::usize metadata_bytes{ 0u };
    common::usize backing_bytes{ 0u };
    common::usize allocation_word_count{ 0u };

    common::usize used_count{ 0u };
    common::usize free_count{ 0u };
    common::usize peak_used_count{ 0u };

    common::u64 allocation_count{ 0u };
    common::u64 free_operation_count{ 0u };
    common::u64 failed_allocation_count{ 0u };
    common::u64 failed_free_count{ 0u };

    common::u32 flags{ CYPHER_MEMORY_POOL_FLAG_NONE };
    pool_backing_t backing{ pool_backing_t::POOL_ARENA };
    error_code_t last_error{ error_code_t::OK };

    pool_operation_trace_t operation_traces[CYPHER_MEMORY_POOL_OPERATION_TRACE_COUNT]{};
    common::usize operation_trace_index{ 0u };
    common::usize operation_trace_count{ 0u };

    bool initialized{ false };
};

using pool_allocator_t = pool_t;

error_code_t CypherMemory_PoolInit( pool_t &pool, const pool_desc_t &pool_desc );

void CypherMemory_PoolShutdown( pool_t &pool );

pool_stats_t CypherMemory_PoolStats( const pool_t &pool );

void CypherMemory_PoolResetCounters( pool_t &pool );

void CypherMemory_PoolReset( pool_t &pool );

void *CypherMemory_PoolAlloc( pool_t &pool );

void *CypherMemory_PoolAllocDebug( pool_t &pool, const char *file, const char *function, common::i32 line );

void *CypherMemory_PoolAllocZero( pool_t &pool );

void *CypherMemory_PoolAllocZeroDebug( pool_t &pool, const char *file, const char *function, common::i32 line );

void *CypherMemory_PoolAllocSize( pool_t &pool, common::usize size, common::usize alignment = CYPHER_MEMORY_DEFAULT_ALIGNMENT );

void *CypherMemory_PoolAllocSizeDebug( pool_t &pool,
                                       common::usize size,
                                       common::usize alignment,
                                       const char *file,
                                       const char *function,
                                       common::i32 line );

void *CypherMemory_PoolAllocSizeZero( pool_t &pool, common::usize size, common::usize alignment = CYPHER_MEMORY_DEFAULT_ALIGNMENT );

void *CypherMemory_PoolAllocSizeZeroDebug( pool_t &pool,
                                           common::usize size,
                                           common::usize alignment,
                                           const char *file,
                                           const char *function,
                                           common::i32 line );

error_code_t CypherMemory_PoolFree( pool_t &pool, void *ptr );

error_code_t CypherMemory_PoolFreeDebug( pool_t &pool, void *ptr, const char *file, const char *function, common::i32 line );

bool CypherMemory_PoolContains( const pool_t &pool, const void *ptr );

bool CypherMemory_PoolOwnsSlot( const pool_t &pool, const void *ptr );

bool CypherMemory_PoolIsInitialized( const pool_t &pool );

error_code_t CypherMemory_PoolLastError( const pool_t &pool );

common::usize CypherMemory_PoolSlotIndex( const pool_t &pool, const void *ptr );

common::usize CypherMemory_PoolUsedCount( const pool_t &pool );

common::usize CypherMemory_PoolFreeCount( const pool_t &pool );

common::usize CypherMemory_PoolCapacity( const pool_t &pool );

common::f32 CypherMemory_PoolUsageRatio( const pool_t &pool );

const pool_operation_trace_t *CypherMemory_PoolOperationTraces( const pool_t &pool, common::usize &out_count );

template <typename T>
T *CypherMemory_PoolAllocType( pool_t &pool )
{
    return static_cast<T *>( CypherMemory_PoolAllocSize( pool, sizeof( T ), alignof( T ) ) );
}

template <typename T>
T *CypherMemory_PoolAllocTypeDebug( pool_t &pool, const char *file, const char *function, common::i32 line )
{
    return static_cast<T *>( CypherMemory_PoolAllocSizeDebug( pool, sizeof( T ), alignof( T ), file, function, line ) );
}

template <typename T>
T *CypherMemory_PoolAllocTypeZero( pool_t &pool )
{
    return static_cast<T *>( CypherMemory_PoolAllocSizeZero( pool, sizeof( T ), alignof( T ) ) );
}

template <typename T>
T *CypherMemory_PoolAllocTypeZeroDebug( pool_t &pool, const char *file, const char *function, common::i32 line )
{
    return static_cast<T *>( CypherMemory_PoolAllocSizeZeroDebug( pool, sizeof( T ), alignof( T ), file, function, line ) );
}

}       // namespace cypher::engine::memory

#define CYPHER_MEMORY_POOL_ALLOC( POOL ) \
    ::cypher::engine::memory::CypherMemory_PoolAllocDebug( ( POOL ), __FILE__, __func__, __LINE__ )

#define CYPHER_MEMORY_POOL_ALLOC_ZERO( POOL ) \
    ::cypher::engine::memory::CypherMemory_PoolAllocZeroDebug( ( POOL ), __FILE__, __func__, __LINE__ )

#define CYPHER_MEMORY_POOL_ALLOC_SIZE( POOL, SIZE, ALIGNMENT ) \
    ::cypher::engine::memory::CypherMemory_PoolAllocSizeDebug( ( POOL ), ( SIZE ), ( ALIGNMENT ), __FILE__, __func__, __LINE__ )

#define CYPHER_MEMORY_POOL_ALLOC_SIZE_ZERO( POOL, SIZE, ALIGNMENT ) \
    ::cypher::engine::memory::CypherMemory_PoolAllocSizeZeroDebug( ( POOL ), ( SIZE ), ( ALIGNMENT ), __FILE__, __func__, __LINE__ )

#define CYPHER_MEMORY_POOL_ALLOC_TYPE( POOL, TYPE ) \
    ::cypher::engine::memory::CypherMemory_PoolAllocTypeDebug<TYPE>( ( POOL ), __FILE__, __func__, __LINE__ )

#define CYPHER_MEMORY_POOL_ALLOC_TYPE_ZERO( POOL, TYPE ) \
    ::cypher::engine::memory::CypherMemory_PoolAllocTypeZeroDebug<TYPE>( ( POOL ), __FILE__, __func__, __LINE__ )

#define CYPHER_MEMORY_POOL_FREE( POOL, PTR ) \
    ::cypher::engine::memory::CypherMemory_PoolFreeDebug( ( POOL ), ( PTR ), __FILE__, __func__, __LINE__ )

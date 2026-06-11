/*======================================================================
   File: CypherMemory_Bucket.cpp
   Project: CYPHER
   Author: ksiric <email@example.com>
   Created: 2026-06-12
   ---------------------------------------------------------------------
   Description:
       Small variable-size bucket allocator built from fixed-block pools.
   ---------------------------------------------------------------------
   License:
   Company:
   Version: 0.1.0
 ======================================================================
                                                                       */

#include "CypherEngine/CypherMemory/CypherMemory_Bucket.h"
#include "CypherEngine/CypherLog/CypherLog.h"

#include <cstring>
#include <limits>

namespace cypher::engine::memory
{

namespace {

constexpr common::usize CYPHER_MEMORY_BUCKET_INVALID_CLASS_INDEX = std::numeric_limits<common::usize>::max();

common::u32 CypherMemory_BucketPoolFlags( const common::u32 bucket_flags )
{
    common::u32 pool_flags = CYPHER_MEMORY_POOL_FLAG_NONE;

    if ( ( bucket_flags & CYPHER_MEMORY_BUCKET_FLAG_ZERO_ON_ALLOC ) != 0u ) {
        pool_flags |= CYPHER_MEMORY_POOL_FLAG_ZERO_ON_ALLOC;
    }

    if ( ( bucket_flags & CYPHER_MEMORY_BUCKET_FLAG_CLEAR_ON_FREE ) != 0u ) {
        pool_flags |= CYPHER_MEMORY_POOL_FLAG_CLEAR_ON_FREE;
    }

    if ( ( bucket_flags & CYPHER_MEMORY_BUCKET_FLAG_CLEAR_ON_RESET ) != 0u ) {
        pool_flags |= CYPHER_MEMORY_POOL_FLAG_CLEAR_ON_RESET;
    }

    if ( ( bucket_flags & CYPHER_MEMORY_BUCKET_FLAG_CLEAR_ON_SHUTDOWN ) != 0u ) {
        pool_flags |= CYPHER_MEMORY_POOL_FLAG_CLEAR_ON_SHUTDOWN;
    }

    return pool_flags;
}

error_code_t CypherMemory_BucketFailInit( bucket_t &bucket,
                                          const bucket_desc_t &bucket_desc,
                                          const error_code_t error,
                                          const char *reason )
{
    bucket.name = bucket_desc.name;
    bucket.last_error = error;

    CYPHER_LOG_ERROR( log::channel_t::MEMORY,
                      "bucket '%s' init failed: %s.",
                      bucket.name ? bucket.name : "<unnamed>",
                      reason ? reason : CypherMemory_ErrorDesc( error ) );

    return error;
}

common::usize CypherMemory_BucketFindBestClass( const bucket_t &bucket,
                                                const common::usize size,
                                                const common::usize alignment,
                                                const bool require_free_slot )
{
    common::usize best_index = CYPHER_MEMORY_BUCKET_INVALID_CLASS_INDEX;
    common::usize best_slot_size = std::numeric_limits<common::usize>::max();

    for ( common::usize class_index = 0u; class_index < bucket.class_count; ++class_index ) {
        const bucket_class_t &bucket_class = bucket.classes[class_index];

        if ( bucket_class.slot_size < size ) {
            continue;
        }

        if ( bucket_class.pool.alignment < alignment ) {
            continue;
        }

        if ( require_free_slot && CypherMemory_PoolFreeCount( bucket_class.pool ) == 0u ) {
            continue;
        }

        if ( bucket_class.slot_size < best_slot_size ) {
            best_index = class_index;
            best_slot_size = bucket_class.slot_size;
        }
    }

    return best_index;
}

bool CypherMemory_BucketHasCompatibleClass( const bucket_t &bucket,
                                            const common::usize size,
                                            const common::usize alignment )
{
    return CypherMemory_BucketFindBestClass( bucket, size, alignment, false ) != CYPHER_MEMORY_BUCKET_INVALID_CLASS_INDEX;
}

void CypherMemory_BucketRefreshPeak( bucket_t &bucket )
{
    const common::usize used_count = CypherMemory_BucketUsedCount( bucket );

    if ( used_count > bucket.peak_used_count ) {
        bucket.peak_used_count = used_count;
    }
}

void *CypherMemory_BucketFailAlloc( bucket_t &bucket,
                                    const error_code_t error,
                                    const char *reason )
{
    bucket.last_error = error;
    ++bucket.failed_allocation_count;

    CYPHER_LOG_ERROR( log::channel_t::MEMORY,
                      "bucket '%s' allocation failed: %s.",
                      bucket.name ? bucket.name : "<unnamed>",
                      reason ? reason : CypherMemory_ErrorDesc( error ) );

    return nullptr;
}

}       // namespace

bucket_desc_t CypherMemory_BucketDefaultDesc( arena_t &arena, const char *name )
{
    bucket_desc_t desc{};

    desc.name = name;
    desc.arena = &arena;
    desc.alignment = 16u;
    desc.class_count = CYPHER_MEMORY_BUCKET_DEFAULT_CLASS_COUNT;
    desc.flags = CYPHER_MEMORY_BUCKET_FLAG_NONE;

    desc.classes[0] = bucket_class_desc_t{ 16u, 1024u };
    desc.classes[1] = bucket_class_desc_t{ 32u, 1024u };
    desc.classes[2] = bucket_class_desc_t{ 64u, 1024u };
    desc.classes[3] = bucket_class_desc_t{ 128u, 512u };
    desc.classes[4] = bucket_class_desc_t{ 256u, 256u };
    desc.classes[5] = bucket_class_desc_t{ 512u, 128u };
    desc.classes[6] = bucket_class_desc_t{ 1024u, 64u };
    desc.classes[7] = bucket_class_desc_t{ 2048u, 32u };
    desc.classes[8] = bucket_class_desc_t{ 4096u, 16u };
    desc.classes[9] = bucket_class_desc_t{ 8192u, 8u };

    return desc;
}

error_code_t CypherMemory_BucketInit( bucket_t &bucket, const bucket_desc_t &bucket_desc )
{
    if ( bucket.initialized ) {
        bucket.last_error = error_code_t::ERR_ALREADY_INITIALIZED;
        CYPHER_LOG_WARNING( log::channel_t::MEMORY, "bucket '%s' is already initialized.", bucket.name ? bucket.name : "<unnamed>" );
        return bucket.last_error;
    }

    if ( bucket_desc.arena == nullptr ) {
        return CypherMemory_BucketFailInit( bucket, bucket_desc, error_code_t::ERR_INVALID_ARGUMENT, "arena pointer is required" );
    }

    if ( !CypherMemory_ArenaIsInitialized( *bucket_desc.arena ) ) {
        return CypherMemory_BucketFailInit( bucket, bucket_desc, error_code_t::ERR_NOT_INITIALIZED, "backing arena is not initialized" );
    }

    if ( bucket_desc.class_count == 0u || bucket_desc.class_count > CYPHER_MEMORY_BUCKET_MAX_CLASSES ) {
        return CypherMemory_BucketFailInit( bucket, bucket_desc, error_code_t::ERR_INVALID_CAPACITY, "invalid bucket class count" );
    }

    if ( !CypherMemory_IsPowerOfTwo( bucket_desc.alignment ) ) {
        return CypherMemory_BucketFailInit( bucket, bucket_desc, error_code_t::ERR_INVALID_ALIGNMENT, "invalid bucket alignment" );
    }

    const arena_marker_t init_marker = CypherMemory_ArenaGetMarker( *bucket_desc.arena );

    bucket = {};
    bucket.name = bucket_desc.name;
    bucket.arena = bucket_desc.arena;
    bucket.class_count = bucket_desc.class_count;
    bucket.alignment = bucket_desc.alignment;
    bucket.flags = bucket_desc.flags;

    const common::u32 pool_flags = CypherMemory_BucketPoolFlags( bucket_desc.flags );

    for ( common::usize class_index = 0u; class_index < bucket_desc.class_count; ++class_index ) {
        const bucket_class_desc_t &class_desc = bucket_desc.classes[class_index];

        if ( class_desc.slot_size == 0u || class_desc.slot_count == 0u ) {
            for ( common::usize shutdown_index = 0u; shutdown_index < class_index; ++shutdown_index ) {
                CypherMemory_PoolShutdown( bucket.classes[shutdown_index].pool );
            }
            CypherMemory_ArenaRewind( *bucket_desc.arena, init_marker );
            bucket = {};
            return CypherMemory_BucketFailInit( bucket, bucket_desc, error_code_t::ERR_INVALID_CAPACITY, "invalid bucket class configuration" );
        }

        bucket_class_t &bucket_class = bucket.classes[class_index];
        bucket_class.slot_size = class_desc.slot_size;
        bucket_class.slot_count = class_desc.slot_count;

        pool_desc_t pool_desc{};
        pool_desc.name = bucket_desc.name;
        pool_desc.arena = bucket_desc.arena;
        pool_desc.slot_size = class_desc.slot_size;
        pool_desc.slot_count = class_desc.slot_count;
        pool_desc.alignment = bucket_desc.alignment;
        pool_desc.flags = pool_flags;
        pool_desc.backing = pool_backing_t::POOL_ARENA;

        const error_code_t pool_result = CypherMemory_PoolInit( bucket_class.pool, pool_desc );
        if ( pool_result != error_code_t::OK ) {
            for ( common::usize shutdown_index = 0u; shutdown_index < class_index; ++shutdown_index ) {
                CypherMemory_PoolShutdown( bucket.classes[shutdown_index].pool );
            }
            CypherMemory_ArenaRewind( *bucket_desc.arena, init_marker );
            bucket = {};
            return CypherMemory_BucketFailInit( bucket, bucket_desc, pool_result, CypherMemory_ErrorDesc( pool_result ) );
        }
    }

    bucket.initialized = true;
    bucket.last_error = error_code_t::OK;

    CYPHER_LOG_INFO( log::channel_t::MEMORY,
                     "bucket '%s' initialized: classes=%zu, alignment=%zu, backing_bytes=%zu.",
                     bucket.name ? bucket.name : "<unnamed>",
                     bucket.class_count,
                     bucket.alignment,
                     CypherMemory_BucketStats( bucket ).backing_bytes );

    return error_code_t::OK;
}

void CypherMemory_BucketShutdown( bucket_t &bucket )
{
    if ( !bucket.initialized ) {
        return;
    }

    CYPHER_LOG_INFO( log::channel_t::MEMORY,
                     "bucket '%s' shutdown: used=%zu, peak=%zu, allocations=%llu, frees=%llu, failed_alloc=%llu, failed_free=%llu.",
                     bucket.name ? bucket.name : "<unnamed>",
                     CypherMemory_BucketUsedCount( bucket ),
                     bucket.peak_used_count,
                     static_cast<unsigned long long>( bucket.allocation_count ),
                     static_cast<unsigned long long>( bucket.free_operation_count ),
                     static_cast<unsigned long long>( bucket.failed_allocation_count ),
                     static_cast<unsigned long long>( bucket.failed_free_count ) );

    for ( common::usize class_index = 0u; class_index < bucket.class_count; ++class_index ) {
        CypherMemory_PoolShutdown( bucket.classes[class_index].pool );
    }

    bucket = {};
}

void CypherMemory_BucketReset( bucket_t &bucket )
{
    if ( !bucket.initialized ) {
        return;
    }

    for ( common::usize class_index = 0u; class_index < bucket.class_count; ++class_index ) {
        CypherMemory_PoolReset( bucket.classes[class_index].pool );
    }

    bucket.last_error = error_code_t::OK;
}

void CypherMemory_BucketResetCounters( bucket_t &bucket )
{
    if ( !bucket.initialized ) {
        return;
    }

    bucket.peak_used_count = CypherMemory_BucketUsedCount( bucket );
    bucket.allocation_count = 0u;
    bucket.free_operation_count = 0u;
    bucket.failed_allocation_count = 0u;
    bucket.failed_free_count = 0u;
    bucket.last_error = error_code_t::OK;

    for ( common::usize class_index = 0u; class_index < bucket.class_count; ++class_index ) {
        CypherMemory_PoolResetCounters( bucket.classes[class_index].pool );
    }
}

bucket_stats_t CypherMemory_BucketStats( const bucket_t &bucket )
{
    bucket_stats_t stats{};

    stats.name = bucket.name;
    stats.class_count = bucket.class_count;
    stats.peak_used_count = bucket.peak_used_count;
    stats.allocation_count = bucket.allocation_count;
    stats.free_operation_count = bucket.free_operation_count;
    stats.failed_allocation_count = bucket.failed_allocation_count;
    stats.failed_free_count = bucket.failed_free_count;

    for ( common::usize class_index = 0u; class_index < bucket.class_count; ++class_index ) {
        const pool_stats_t pool_stats = CypherMemory_PoolStats( bucket.classes[class_index].pool );
        bucket_class_stats_t &class_stats = stats.class_stats[class_index];

        class_stats.slot_size = pool_stats.slot_size;
        class_stats.slot_count = pool_stats.slot_count;
        class_stats.used_count = pool_stats.used_count;
        class_stats.free_count = pool_stats.free_count;
        class_stats.peak_used_count = pool_stats.peak_used_count;
        class_stats.backing_bytes = pool_stats.backing_bytes;

        stats.used_count += pool_stats.used_count;
        stats.free_count += pool_stats.free_count;
        stats.backing_bytes += pool_stats.backing_bytes;
    }

    return stats;
}

void *CypherMemory_BucketAlloc( bucket_t &bucket, common::usize size, common::usize alignment )
{
    return CypherMemory_BucketAllocDebug( bucket, size, alignment, nullptr, nullptr, 0 );
}

void *CypherMemory_BucketAllocDebug( bucket_t &bucket,
                                     common::usize size,
                                     common::usize alignment,
                                     const char *file,
                                     const char *function,
                                     common::i32 line )
{
    if ( !bucket.initialized ) {
        return CypherMemory_BucketFailAlloc( bucket, error_code_t::ERR_NOT_INITIALIZED, "bucket is not initialized" );
    }

    if ( size == 0u ) {
        return CypherMemory_BucketFailAlloc( bucket, error_code_t::ERR_INVALID_ARGUMENT, "requested size is zero" );
    }

    if ( !CypherMemory_IsPowerOfTwo( alignment ) ) {
        return CypherMemory_BucketFailAlloc( bucket, error_code_t::ERR_INVALID_ALIGNMENT, "requested alignment is invalid" );
    }

    if ( !CypherMemory_BucketHasCompatibleClass( bucket, size, alignment ) ) {
        return CypherMemory_BucketFailAlloc( bucket, error_code_t::ERR_BUFFER_TOO_SMALL, "no bucket class can satisfy the request" );
    }

    const common::usize class_index = CypherMemory_BucketFindBestClass( bucket, size, alignment, true );
    if ( class_index == CYPHER_MEMORY_BUCKET_INVALID_CLASS_INDEX ) {
        return CypherMemory_BucketFailAlloc( bucket, error_code_t::ERR_OUT_OF_MEMORY, "all compatible bucket classes are full" );
    }

    void *memory = CypherMemory_PoolAllocSizeDebug( bucket.classes[class_index].pool, size, alignment, file, function, line );
    bucket.last_error = CypherMemory_PoolLastError( bucket.classes[class_index].pool );

    if ( memory == nullptr ) {
        ++bucket.failed_allocation_count;
        return nullptr;
    }

    ++bucket.allocation_count;
    CypherMemory_BucketRefreshPeak( bucket );

    return memory;
}

void *CypherMemory_BucketAllocZero( bucket_t &bucket, common::usize size, common::usize alignment )
{
    return CypherMemory_BucketAllocZeroDebug( bucket, size, alignment, nullptr, nullptr, 0 );
}

void *CypherMemory_BucketAllocZeroDebug( bucket_t &bucket,
                                         common::usize size,
                                         common::usize alignment,
                                         const char *file,
                                         const char *function,
                                         common::i32 line )
{
    void *memory = CypherMemory_BucketAllocDebug( bucket, size, alignment, file, function, line );

    if ( memory == nullptr ) {
        return nullptr;
    }

    for ( common::usize class_index = 0u; class_index < bucket.class_count; ++class_index ) {
        const pool_t &pool = bucket.classes[class_index].pool;
        if ( CypherMemory_PoolOwnsSlot( pool, memory ) ) {
            std::memset( memory, 0, pool.slot_stride );
            break;
        }
    }

    return memory;
}

error_code_t CypherMemory_BucketFree( bucket_t &bucket, void *ptr )
{
    return CypherMemory_BucketFreeDebug( bucket, ptr, nullptr, nullptr, 0 );
}

error_code_t CypherMemory_BucketFreeDebug( bucket_t &bucket, void *ptr, const char *file, const char *function, common::i32 line )
{
    if ( !bucket.initialized ) {
        bucket.last_error = error_code_t::ERR_NOT_INITIALIZED;
        ++bucket.failed_free_count;
        return bucket.last_error;
    }

    if ( ptr == nullptr ) {
        bucket.last_error = error_code_t::ERR_INVALID_POINTER;
        ++bucket.failed_free_count;
        return bucket.last_error;
    }

    for ( common::usize class_index = 0u; class_index < bucket.class_count; ++class_index ) {
        pool_t &pool = bucket.classes[class_index].pool;
        if ( !CypherMemory_PoolOwnsSlot( pool, ptr ) ) {
            continue;
        }

        const error_code_t free_result = CypherMemory_PoolFreeDebug( pool, ptr, file, function, line );
        bucket.last_error = free_result;

        if ( free_result == error_code_t::OK ) {
            ++bucket.free_operation_count;
        } else {
            ++bucket.failed_free_count;
        }

        return free_result;
    }

    bucket.last_error = error_code_t::ERR_INVALID_POINTER;
    ++bucket.failed_free_count;
    CYPHER_LOG_ERROR( log::channel_t::MEMORY,
                      "bucket '%s' free failed: pointer does not belong to any bucket class.",
                      bucket.name ? bucket.name : "<unnamed>" );

    return bucket.last_error;
}

bool CypherMemory_BucketContains( const bucket_t &bucket, const void *ptr )
{
    if ( !bucket.initialized || ptr == nullptr ) {
        return false;
    }

    for ( common::usize class_index = 0u; class_index < bucket.class_count; ++class_index ) {
        if ( CypherMemory_PoolContains( bucket.classes[class_index].pool, ptr ) ) {
            return true;
        }
    }

    return false;
}

bool CypherMemory_BucketOwnsSlot( const bucket_t &bucket, const void *ptr )
{
    if ( !bucket.initialized || ptr == nullptr ) {
        return false;
    }

    for ( common::usize class_index = 0u; class_index < bucket.class_count; ++class_index ) {
        if ( CypherMemory_PoolOwnsSlot( bucket.classes[class_index].pool, ptr ) ) {
            return true;
        }
    }

    return false;
}

bool CypherMemory_BucketIsInitialized( const bucket_t &bucket )
{
    return bucket.initialized;
}

error_code_t CypherMemory_BucketLastError( const bucket_t &bucket )
{
    return bucket.last_error;
}

common::usize CypherMemory_BucketClassIndexForSize( const bucket_t &bucket, common::usize size, common::usize alignment )
{
    if ( !bucket.initialized || size == 0u || !CypherMemory_IsPowerOfTwo( alignment ) ) {
        return CYPHER_MEMORY_BUCKET_INVALID_CLASS_INDEX;
    }

    return CypherMemory_BucketFindBestClass( bucket, size, alignment, false );
}

common::usize CypherMemory_BucketUsedCount( const bucket_t &bucket )
{
    common::usize used_count = 0u;

    for ( common::usize class_index = 0u; class_index < bucket.class_count; ++class_index ) {
        used_count += CypherMemory_PoolUsedCount( bucket.classes[class_index].pool );
    }

    return used_count;
}

common::usize CypherMemory_BucketFreeCount( const bucket_t &bucket )
{
    common::usize free_count = 0u;

    for ( common::usize class_index = 0u; class_index < bucket.class_count; ++class_index ) {
        free_count += CypherMemory_PoolFreeCount( bucket.classes[class_index].pool );
    }

    return free_count;
}

common::f32 CypherMemory_BucketUsageRatio( const bucket_t &bucket )
{
    const common::usize used_count = CypherMemory_BucketUsedCount( bucket );
    const common::usize free_count = CypherMemory_BucketFreeCount( bucket );
    const common::usize total_count = used_count + free_count;

    if ( total_count == 0u ) {
        return 0.0f;
    }

    return static_cast<common::f32>( used_count ) / static_cast<common::f32>( total_count );
}

}       // namespace cypher::engine::memory

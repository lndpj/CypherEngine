/*======================================================================
   File: CypherMemory_Thread.cpp
   Project: CYPHER
   Author: ksiric <email@example.com>
   Created: 2026-06-12
   ---------------------------------------------------------------------
   Description:
       Explicit thread-safe wrappers for shared memory allocators.
   ---------------------------------------------------------------------
   License:
   Company:
   Version: 0.1.0
 ======================================================================
                                                                       */

#include "CypherEngine/CypherMemory/CypherMemory_Thread.h"
#include "CypherEngine/CypherLog/CypherLog.h"

namespace cypher::engine::memory
{

namespace {

void *CypherMemory_ThreadSafeAllocFail( const char *allocator_name, const error_code_t error, const char *reason )
{
    LOG_ERROR( log::channel_t::MEMORY,
                      "thread-safe allocator '%s' allocation failed: %s.",
                      allocator_name ? allocator_name : "<unnamed>",
                      reason ? reason : CypherMemory_ErrorDesc( error ) );
    return nullptr;
}

}       // namespace

void CypherMemory_MutexLock( memory_mutex_t &mutex )
{
    mutex.native_mutex.lock();
}

void CypherMemory_MutexUnlock( memory_mutex_t &mutex )
{
    mutex.native_mutex.unlock();
}

error_code_t CypherMemory_ThreadSafeArenaBind( thread_safe_arena_t &thread_safe_arena, arena_t &arena )
{
    if ( thread_safe_arena.initialized ) {
        thread_safe_arena.last_error = error_code_t::ERR_ALREADY_INITIALIZED;
        return thread_safe_arena.last_error;
    }

    if ( !CypherMemory_ArenaIsInitialized( arena ) ) {
        thread_safe_arena.last_error = error_code_t::ERR_NOT_INITIALIZED;
        return thread_safe_arena.last_error;
    }

    thread_safe_arena.arena = &arena;
    thread_safe_arena.last_error = error_code_t::OK;
    thread_safe_arena.initialized = true;

    return thread_safe_arena.last_error;
}

void CypherMemory_ThreadSafeArenaUnbind( thread_safe_arena_t &thread_safe_arena )
{
    CypherMemory_MutexLock( thread_safe_arena.mutex );
    thread_safe_arena.arena = nullptr;
    thread_safe_arena.last_error = error_code_t::OK;
    thread_safe_arena.initialized = false;
    CypherMemory_MutexUnlock( thread_safe_arena.mutex );
}

void *CypherMemory_ThreadSafeArenaAlloc( thread_safe_arena_t &thread_safe_arena,
                                         common::usize size,
                                         common::usize alignment )
{
    return CypherMemory_ThreadSafeArenaAllocDebug( thread_safe_arena, size, alignment, nullptr, nullptr, 0 );
}

void *CypherMemory_ThreadSafeArenaAllocDebug( thread_safe_arena_t &thread_safe_arena,
                                              common::usize size,
                                              common::usize alignment,
                                              const char *file,
                                              const char *function,
                                              common::i32 line )
{
    if ( !thread_safe_arena.initialized || thread_safe_arena.arena == nullptr ) {
        thread_safe_arena.last_error = error_code_t::ERR_NOT_INITIALIZED;
        return CypherMemory_ThreadSafeAllocFail( nullptr, thread_safe_arena.last_error, "arena wrapper is not initialized" );
    }

    CypherMemory_MutexLock( thread_safe_arena.mutex );
    void *memory = CypherMemory_ArenaAllocDebug( *thread_safe_arena.arena, size, alignment, file, function, line );
    thread_safe_arena.last_error = CypherMemory_ArenaLastError( *thread_safe_arena.arena );
    CypherMemory_MutexUnlock( thread_safe_arena.mutex );

    return memory;
}

void *CypherMemory_ThreadSafeArenaAllocZero( thread_safe_arena_t &thread_safe_arena,
                                             common::usize size,
                                             common::usize alignment )
{
    return CypherMemory_ThreadSafeArenaAllocZeroDebug( thread_safe_arena, size, alignment, nullptr, nullptr, 0 );
}

void *CypherMemory_ThreadSafeArenaAllocZeroDebug( thread_safe_arena_t &thread_safe_arena,
                                                  common::usize size,
                                                  common::usize alignment,
                                                  const char *file,
                                                  const char *function,
                                                  common::i32 line )
{
    if ( !thread_safe_arena.initialized || thread_safe_arena.arena == nullptr ) {
        thread_safe_arena.last_error = error_code_t::ERR_NOT_INITIALIZED;
        return CypherMemory_ThreadSafeAllocFail( nullptr, thread_safe_arena.last_error, "arena wrapper is not initialized" );
    }

    CypherMemory_MutexLock( thread_safe_arena.mutex );
    void *memory = CypherMemory_ArenaAllocZeroDebug( *thread_safe_arena.arena, size, alignment, file, function, line );
    thread_safe_arena.last_error = CypherMemory_ArenaLastError( *thread_safe_arena.arena );
    CypherMemory_MutexUnlock( thread_safe_arena.mutex );

    return memory;
}

void CypherMemory_ThreadSafeArenaReset( thread_safe_arena_t &thread_safe_arena )
{
    if ( !thread_safe_arena.initialized || thread_safe_arena.arena == nullptr ) {
        thread_safe_arena.last_error = error_code_t::ERR_NOT_INITIALIZED;
        return;
    }

    CypherMemory_MutexLock( thread_safe_arena.mutex );
    CypherMemory_ArenaReset( *thread_safe_arena.arena );
    thread_safe_arena.last_error = CypherMemory_ArenaLastError( *thread_safe_arena.arena );
    CypherMemory_MutexUnlock( thread_safe_arena.mutex );
}

arena_stats_t CypherMemory_ThreadSafeArenaStats( thread_safe_arena_t &thread_safe_arena )
{
    arena_stats_t stats{};

    if ( !thread_safe_arena.initialized || thread_safe_arena.arena == nullptr ) {
        thread_safe_arena.last_error = error_code_t::ERR_NOT_INITIALIZED;
        return stats;
    }

    CypherMemory_MutexLock( thread_safe_arena.mutex );
    stats = CypherMemory_ArenaStats( *thread_safe_arena.arena );
    thread_safe_arena.last_error = CypherMemory_ArenaLastError( *thread_safe_arena.arena );
    CypherMemory_MutexUnlock( thread_safe_arena.mutex );

    return stats;
}

error_code_t CypherMemory_ThreadSafeArenaLastError( const thread_safe_arena_t &thread_safe_arena )
{
    return thread_safe_arena.last_error;
}

error_code_t CypherMemory_ThreadSafePoolBind( thread_safe_pool_t &thread_safe_pool, pool_t &pool )
{
    if ( thread_safe_pool.initialized ) {
        thread_safe_pool.last_error = error_code_t::ERR_ALREADY_INITIALIZED;
        return thread_safe_pool.last_error;
    }

    if ( !CypherMemory_PoolIsInitialized( pool ) ) {
        thread_safe_pool.last_error = error_code_t::ERR_NOT_INITIALIZED;
        return thread_safe_pool.last_error;
    }

    thread_safe_pool.pool = &pool;
    thread_safe_pool.last_error = error_code_t::OK;
    thread_safe_pool.initialized = true;

    return thread_safe_pool.last_error;
}

void CypherMemory_ThreadSafePoolUnbind( thread_safe_pool_t &thread_safe_pool )
{
    CypherMemory_MutexLock( thread_safe_pool.mutex );
    thread_safe_pool.pool = nullptr;
    thread_safe_pool.last_error = error_code_t::OK;
    thread_safe_pool.initialized = false;
    CypherMemory_MutexUnlock( thread_safe_pool.mutex );
}

void *CypherMemory_ThreadSafePoolAlloc( thread_safe_pool_t &thread_safe_pool )
{
    return CypherMemory_ThreadSafePoolAllocDebug( thread_safe_pool, nullptr, nullptr, 0 );
}

void *CypherMemory_ThreadSafePoolAllocDebug( thread_safe_pool_t &thread_safe_pool,
                                             const char *file,
                                             const char *function,
                                             common::i32 line )
{
    if ( !thread_safe_pool.initialized || thread_safe_pool.pool == nullptr ) {
        thread_safe_pool.last_error = error_code_t::ERR_NOT_INITIALIZED;
        return CypherMemory_ThreadSafeAllocFail( nullptr, thread_safe_pool.last_error, "pool wrapper is not initialized" );
    }

    CypherMemory_MutexLock( thread_safe_pool.mutex );
    void *memory = CypherMemory_PoolAllocDebug( *thread_safe_pool.pool, file, function, line );
    thread_safe_pool.last_error = CypherMemory_PoolLastError( *thread_safe_pool.pool );
    CypherMemory_MutexUnlock( thread_safe_pool.mutex );

    return memory;
}

void *CypherMemory_ThreadSafePoolAllocZero( thread_safe_pool_t &thread_safe_pool )
{
    return CypherMemory_ThreadSafePoolAllocZeroDebug( thread_safe_pool, nullptr, nullptr, 0 );
}

void *CypherMemory_ThreadSafePoolAllocZeroDebug( thread_safe_pool_t &thread_safe_pool,
                                                 const char *file,
                                                 const char *function,
                                                 common::i32 line )
{
    if ( !thread_safe_pool.initialized || thread_safe_pool.pool == nullptr ) {
        thread_safe_pool.last_error = error_code_t::ERR_NOT_INITIALIZED;
        return CypherMemory_ThreadSafeAllocFail( nullptr, thread_safe_pool.last_error, "pool wrapper is not initialized" );
    }

    CypherMemory_MutexLock( thread_safe_pool.mutex );
    void *memory = CypherMemory_PoolAllocZeroDebug( *thread_safe_pool.pool, file, function, line );
    thread_safe_pool.last_error = CypherMemory_PoolLastError( *thread_safe_pool.pool );
    CypherMemory_MutexUnlock( thread_safe_pool.mutex );

    return memory;
}

error_code_t CypherMemory_ThreadSafePoolFree( thread_safe_pool_t &thread_safe_pool, void *ptr )
{
    return CypherMemory_ThreadSafePoolFreeDebug( thread_safe_pool, ptr, nullptr, nullptr, 0 );
}

error_code_t CypherMemory_ThreadSafePoolFreeDebug( thread_safe_pool_t &thread_safe_pool,
                                                   void *ptr,
                                                   const char *file,
                                                   const char *function,
                                                   common::i32 line )
{
    if ( !thread_safe_pool.initialized || thread_safe_pool.pool == nullptr ) {
        thread_safe_pool.last_error = error_code_t::ERR_NOT_INITIALIZED;
        return thread_safe_pool.last_error;
    }

    CypherMemory_MutexLock( thread_safe_pool.mutex );
    thread_safe_pool.last_error = CypherMemory_PoolFreeDebug( *thread_safe_pool.pool, ptr, file, function, line );
    CypherMemory_MutexUnlock( thread_safe_pool.mutex );

    return thread_safe_pool.last_error;
}

void CypherMemory_ThreadSafePoolReset( thread_safe_pool_t &thread_safe_pool )
{
    if ( !thread_safe_pool.initialized || thread_safe_pool.pool == nullptr ) {
        thread_safe_pool.last_error = error_code_t::ERR_NOT_INITIALIZED;
        return;
    }

    CypherMemory_MutexLock( thread_safe_pool.mutex );
    CypherMemory_PoolReset( *thread_safe_pool.pool );
    thread_safe_pool.last_error = CypherMemory_PoolLastError( *thread_safe_pool.pool );
    CypherMemory_MutexUnlock( thread_safe_pool.mutex );
}

pool_stats_t CypherMemory_ThreadSafePoolStats( thread_safe_pool_t &thread_safe_pool )
{
    pool_stats_t stats{};

    if ( !thread_safe_pool.initialized || thread_safe_pool.pool == nullptr ) {
        thread_safe_pool.last_error = error_code_t::ERR_NOT_INITIALIZED;
        return stats;
    }

    CypherMemory_MutexLock( thread_safe_pool.mutex );
    stats = CypherMemory_PoolStats( *thread_safe_pool.pool );
    thread_safe_pool.last_error = CypherMemory_PoolLastError( *thread_safe_pool.pool );
    CypherMemory_MutexUnlock( thread_safe_pool.mutex );

    return stats;
}

error_code_t CypherMemory_ThreadSafePoolLastError( const thread_safe_pool_t &thread_safe_pool )
{
    return thread_safe_pool.last_error;
}

error_code_t CypherMemory_ThreadSafeBucketBind( thread_safe_bucket_t &thread_safe_bucket, bucket_t &bucket )
{
    if ( thread_safe_bucket.initialized ) {
        thread_safe_bucket.last_error = error_code_t::ERR_ALREADY_INITIALIZED;
        return thread_safe_bucket.last_error;
    }

    if ( !CypherMemory_BucketIsInitialized( bucket ) ) {
        thread_safe_bucket.last_error = error_code_t::ERR_NOT_INITIALIZED;
        return thread_safe_bucket.last_error;
    }

    thread_safe_bucket.bucket = &bucket;
    thread_safe_bucket.last_error = error_code_t::OK;
    thread_safe_bucket.initialized = true;

    return thread_safe_bucket.last_error;
}

void CypherMemory_ThreadSafeBucketUnbind( thread_safe_bucket_t &thread_safe_bucket )
{
    CypherMemory_MutexLock( thread_safe_bucket.mutex );
    thread_safe_bucket.bucket = nullptr;
    thread_safe_bucket.last_error = error_code_t::OK;
    thread_safe_bucket.initialized = false;
    CypherMemory_MutexUnlock( thread_safe_bucket.mutex );
}

void *CypherMemory_ThreadSafeBucketAlloc( thread_safe_bucket_t &thread_safe_bucket,
                                          common::usize size,
                                          common::usize alignment )
{
    return CypherMemory_ThreadSafeBucketAllocDebug( thread_safe_bucket, size, alignment, nullptr, nullptr, 0 );
}

void *CypherMemory_ThreadSafeBucketAllocDebug( thread_safe_bucket_t &thread_safe_bucket,
                                               common::usize size,
                                               common::usize alignment,
                                               const char *file,
                                               const char *function,
                                               common::i32 line )
{
    if ( !thread_safe_bucket.initialized || thread_safe_bucket.bucket == nullptr ) {
        thread_safe_bucket.last_error = error_code_t::ERR_NOT_INITIALIZED;
        return CypherMemory_ThreadSafeAllocFail( nullptr, thread_safe_bucket.last_error, "bucket wrapper is not initialized" );
    }

    CypherMemory_MutexLock( thread_safe_bucket.mutex );
    void *memory = CypherMemory_BucketAllocDebug( *thread_safe_bucket.bucket, size, alignment, file, function, line );
    thread_safe_bucket.last_error = CypherMemory_BucketLastError( *thread_safe_bucket.bucket );
    CypherMemory_MutexUnlock( thread_safe_bucket.mutex );

    return memory;
}

void *CypherMemory_ThreadSafeBucketAllocZero( thread_safe_bucket_t &thread_safe_bucket,
                                              common::usize size,
                                              common::usize alignment )
{
    return CypherMemory_ThreadSafeBucketAllocZeroDebug( thread_safe_bucket, size, alignment, nullptr, nullptr, 0 );
}

void *CypherMemory_ThreadSafeBucketAllocZeroDebug( thread_safe_bucket_t &thread_safe_bucket,
                                                   common::usize size,
                                                   common::usize alignment,
                                                   const char *file,
                                                   const char *function,
                                                   common::i32 line )
{
    if ( !thread_safe_bucket.initialized || thread_safe_bucket.bucket == nullptr ) {
        thread_safe_bucket.last_error = error_code_t::ERR_NOT_INITIALIZED;
        return CypherMemory_ThreadSafeAllocFail( nullptr, thread_safe_bucket.last_error, "bucket wrapper is not initialized" );
    }

    CypherMemory_MutexLock( thread_safe_bucket.mutex );
    void *memory = CypherMemory_BucketAllocZeroDebug( *thread_safe_bucket.bucket, size, alignment, file, function, line );
    thread_safe_bucket.last_error = CypherMemory_BucketLastError( *thread_safe_bucket.bucket );
    CypherMemory_MutexUnlock( thread_safe_bucket.mutex );

    return memory;
}

error_code_t CypherMemory_ThreadSafeBucketFree( thread_safe_bucket_t &thread_safe_bucket, void *ptr )
{
    return CypherMemory_ThreadSafeBucketFreeDebug( thread_safe_bucket, ptr, nullptr, nullptr, 0 );
}

error_code_t CypherMemory_ThreadSafeBucketFreeDebug( thread_safe_bucket_t &thread_safe_bucket,
                                                     void *ptr,
                                                     const char *file,
                                                     const char *function,
                                                     common::i32 line )
{
    if ( !thread_safe_bucket.initialized || thread_safe_bucket.bucket == nullptr ) {
        thread_safe_bucket.last_error = error_code_t::ERR_NOT_INITIALIZED;
        return thread_safe_bucket.last_error;
    }

    CypherMemory_MutexLock( thread_safe_bucket.mutex );
    thread_safe_bucket.last_error = CypherMemory_BucketFreeDebug( *thread_safe_bucket.bucket, ptr, file, function, line );
    CypherMemory_MutexUnlock( thread_safe_bucket.mutex );

    return thread_safe_bucket.last_error;
}

void CypherMemory_ThreadSafeBucketReset( thread_safe_bucket_t &thread_safe_bucket )
{
    if ( !thread_safe_bucket.initialized || thread_safe_bucket.bucket == nullptr ) {
        thread_safe_bucket.last_error = error_code_t::ERR_NOT_INITIALIZED;
        return;
    }

    CypherMemory_MutexLock( thread_safe_bucket.mutex );
    CypherMemory_BucketReset( *thread_safe_bucket.bucket );
    thread_safe_bucket.last_error = CypherMemory_BucketLastError( *thread_safe_bucket.bucket );
    CypherMemory_MutexUnlock( thread_safe_bucket.mutex );
}

bucket_stats_t CypherMemory_ThreadSafeBucketStats( thread_safe_bucket_t &thread_safe_bucket )
{
    bucket_stats_t stats{};

    if ( !thread_safe_bucket.initialized || thread_safe_bucket.bucket == nullptr ) {
        thread_safe_bucket.last_error = error_code_t::ERR_NOT_INITIALIZED;
        return stats;
    }

    CypherMemory_MutexLock( thread_safe_bucket.mutex );
    stats = CypherMemory_BucketStats( *thread_safe_bucket.bucket );
    thread_safe_bucket.last_error = CypherMemory_BucketLastError( *thread_safe_bucket.bucket );
    CypherMemory_MutexUnlock( thread_safe_bucket.mutex );

    return stats;
}

error_code_t CypherMemory_ThreadSafeBucketLastError( const thread_safe_bucket_t &thread_safe_bucket )
{
    return thread_safe_bucket.last_error;
}

}       // namespace cypher::engine::memory

#include "CypherEngine/CypherMemory/CypherMemory.h"
#include "CypherEngine/CypherLog/CypherLog.h"
#include "CypherEngine/CypherSystem/CypherSystem_Platform.h"

#include <atomic>
#include <cstdint>
#include <cstdio>
#include <new>
#include <thread>

namespace cypher::engine::log
{

bool CypherLog_LevelEnabled( const level_t, const channel_t )
{
    return false;
}

void CypherLog_Emitf( const level_t,
                      const channel_t,
                      const char *,
                      const char *,
                      const common::com_i32,
                      const char *,
                      ... )
{
}

}       // namespace cypher::engine::log

namespace cypher::engine::sys
{

common::usize CypherSystem_VirtualPageSize()
{
    return 4096u;
}

void *CypherSystem_VirtualReserve( const common::usize size )
{
    return ::operator new( size, std::nothrow );
}

error_code_t CypherSystem_VirtualCommit( void *, common::usize )
{
    return error_code_t::OK;
}

error_code_t CypherSystem_VirtualDecommit( void *, common::usize )
{
    return error_code_t::OK;
}

error_code_t CypherSystem_VirtualRelease( void *memory, common::usize )
{
    ::operator delete( memory );
    return error_code_t::OK;
}

}       // namespace cypher::engine::sys

namespace {

using namespace cypher::engine;

bool SmokeArenaAndScratch()
{
    alignas( 64 ) common::byte arena_buffer[64u * 1024u]{};

    memory::arena_t arena{};
    memory::arena_desc_t arena_desc{};
    arena_desc.name = "SmokeArena";
    arena_desc.capacity = sizeof( arena_buffer );
    arena_desc.external_buffer = arena_buffer;
    arena_desc.backing = memory::arena_backing_t::ARENA_EXTERNAL_BUFFER;

    if ( memory::CypherMemory_ArenaInit( arena, arena_desc ) != memory::error_code_t::OK ) {
        return false;
    }

    void *first = memory::CypherMemory_ArenaAlloc( arena, 128u, 16u );
    if ( first == nullptr || ( reinterpret_cast<std::uintptr_t>( first ) % 16u ) != 0u ) {
        return false;
    }

    memory::scratch_scope_t scratch{};
    if ( memory::CypherMemory_ScratchBegin( scratch, arena, "SmokeScratch" ) != memory::error_code_t::OK ) {
        return false;
    }

    const common::usize used_before_scratch = memory::CypherMemory_ArenaUsed( arena );
    void *scratch_memory = memory::CypherMemory_ScratchAlloc( scratch, 512u, 32u );
    if ( scratch_memory == nullptr || memory::CypherMemory_ArenaUsed( arena ) <= used_before_scratch ) {
        return false;
    }

    if ( memory::CypherMemory_ScratchEnd( scratch ) != memory::error_code_t::OK ) {
        return false;
    }

    if ( memory::CypherMemory_ArenaUsed( arena ) != used_before_scratch ) {
        return false;
    }

    memory::CypherMemory_ArenaShutdown( arena );
    return true;
}

bool SmokePool()
{
    alignas( 64 ) common::byte pool_buffer[4096u]{};

    memory::pool_t pool{};
    memory::pool_desc_t pool_desc{};
    pool_desc.name = "SmokePool";
    pool_desc.external_buffer = pool_buffer;
    pool_desc.external_buffer_size = sizeof( pool_buffer );
    pool_desc.slot_size = 32u;
    pool_desc.slot_count = 16u;
    pool_desc.alignment = 16u;
    pool_desc.backing = memory::pool_backing_t::POOL_EXTERNAL_BUFFER;
    pool_desc.flags = memory::CYPHER_MEMORY_POOL_FLAG_CLEAR_ON_FREE;

    if ( memory::CypherMemory_PoolInit( pool, pool_desc ) != memory::error_code_t::OK ) {
        return false;
    }

    void *slots[16u]{};
    for ( common::usize slot_index = 0u; slot_index < 16u; ++slot_index ) {
        slots[slot_index] = memory::CypherMemory_PoolAlloc( pool );
        if ( slots[slot_index] == nullptr || !memory::CypherMemory_PoolOwnsSlot( pool, slots[slot_index] ) ) {
            return false;
        }
    }

    if ( memory::CypherMemory_PoolAlloc( pool ) != nullptr ) {
        return false;
    }

    if ( memory::CypherMemory_PoolLastError( pool ) != memory::error_code_t::ERR_OUT_OF_MEMORY ) {
        return false;
    }

    if ( memory::CypherMemory_PoolFree( pool, slots[4u] ) != memory::error_code_t::OK ) {
        return false;
    }

    if ( memory::CypherMemory_PoolFree( pool, slots[4u] ) != memory::error_code_t::ERR_DOUBLE_FREE ) {
        return false;
    }

    memory::CypherMemory_PoolShutdown( pool );
    return true;
}

bool SmokeBucket()
{
    alignas( 64 ) common::byte arena_buffer[128u * 1024u]{};

    memory::arena_t arena{};
    memory::arena_desc_t arena_desc{};
    arena_desc.name = "BucketArena";
    arena_desc.capacity = sizeof( arena_buffer );
    arena_desc.external_buffer = arena_buffer;
    arena_desc.backing = memory::arena_backing_t::ARENA_EXTERNAL_BUFFER;

    if ( memory::CypherMemory_ArenaInit( arena, arena_desc ) != memory::error_code_t::OK ) {
        return false;
    }

    memory::bucket_t bucket{};
    memory::bucket_desc_t bucket_desc{};
    bucket_desc.name = "SmokeBucket";
    bucket_desc.arena = &arena;
    bucket_desc.alignment = 16u;
    bucket_desc.class_count = 3u;
    bucket_desc.classes[0] = memory::bucket_class_desc_t{ 32u, 8u };
    bucket_desc.classes[1] = memory::bucket_class_desc_t{ 128u, 4u };
    bucket_desc.classes[2] = memory::bucket_class_desc_t{ 512u, 2u };

    if ( memory::CypherMemory_BucketInit( bucket, bucket_desc ) != memory::error_code_t::OK ) {
        return false;
    }

    void *small = memory::CypherMemory_BucketAlloc( bucket, 24u, 16u );
    void *medium = memory::CypherMemory_BucketAlloc( bucket, 100u, 16u );
    void *large = memory::CypherMemory_BucketAllocZero( bucket, 400u, 16u );

    if ( small == nullptr || medium == nullptr || large == nullptr ) {
        return false;
    }

    if ( !memory::CypherMemory_BucketContains( bucket, small ) ||
         !memory::CypherMemory_BucketContains( bucket, medium ) ||
         !memory::CypherMemory_BucketContains( bucket, large ) ) {
        return false;
    }

    if ( memory::CypherMemory_BucketFree( bucket, medium ) != memory::error_code_t::OK ) {
        return false;
    }

    if ( memory::CypherMemory_BucketFree( bucket, medium ) != memory::error_code_t::ERR_DOUBLE_FREE ) {
        return false;
    }

    memory::CypherMemory_BucketShutdown( bucket );
    memory::CypherMemory_ArenaShutdown( arena );
    return true;
}

bool SmokeThreadSafePool()
{
    alignas( 64 ) common::byte pool_buffer[8192u]{};

    memory::pool_t pool{};
    memory::pool_desc_t pool_desc{};
    pool_desc.name = "ThreadSafePool";
    pool_desc.external_buffer = pool_buffer;
    pool_desc.external_buffer_size = sizeof( pool_buffer );
    pool_desc.slot_size = 32u;
    pool_desc.slot_count = 64u;
    pool_desc.alignment = 16u;
    pool_desc.backing = memory::pool_backing_t::POOL_EXTERNAL_BUFFER;

    if ( memory::CypherMemory_PoolInit( pool, pool_desc ) != memory::error_code_t::OK ) {
        return false;
    }

    memory::thread_safe_pool_t thread_safe_pool{};
    if ( memory::CypherMemory_ThreadSafePoolBind( thread_safe_pool, pool ) != memory::error_code_t::OK ) {
        return false;
    }

    std::atomic<bool> failed{ false };

    auto worker = [&thread_safe_pool, &failed]() {
        for ( common::usize iteration = 0u; iteration < 256u; ++iteration ) {
            void *ptr = memory::CypherMemory_ThreadSafePoolAlloc( thread_safe_pool );
            if ( ptr == nullptr ) {
                failed = true;
                return;
            }

            if ( memory::CypherMemory_ThreadSafePoolFree( thread_safe_pool, ptr ) != memory::error_code_t::OK ) {
                failed = true;
                return;
            }
        }
    };

    std::thread t0( worker );
    std::thread t1( worker );
    std::thread t2( worker );
    std::thread t3( worker );

    t0.join();
    t1.join();
    t2.join();
    t3.join();

    const bool result = !failed && memory::CypherMemory_PoolUsedCount( pool ) == 0u;

    memory::CypherMemory_ThreadSafePoolUnbind( thread_safe_pool );
    memory::CypherMemory_PoolShutdown( pool );

    return result;
}

}       // namespace

int main()
{
    if ( !SmokeArenaAndScratch() ) {
        std::printf( "SmokeArenaAndScratch failed\n" );
        return 1;
    }

    if ( !SmokePool() ) {
        std::printf( "SmokePool failed\n" );
        return 2;
    }

    if ( !SmokeBucket() ) {
        std::printf( "SmokeBucket failed\n" );
        return 3;
    }

    if ( !SmokeThreadSafePool() ) {
        std::printf( "SmokeThreadSafePool failed\n" );
        return 4;
    }

    std::printf( "CypherMemory smoke tests passed\n" );
    return 0;
}

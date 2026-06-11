#pragma once

#include "CypherEngine/CypherCommon/CypherCommon.h"
#include "CypherEngine/CypherMemory/CypherMemory_Error.h"
#include <cassert>
#include <limits>

namespace cypher::engine::memory
{
    
/*
================
Memory Size Helpers
================
*/   
constexpr inline common::usize CypherMemory_Kilobytes( const common::usize value ) 
{
    return value * 1024u;
}

constexpr inline common::usize CypherMemory_Megabytes( const common::usize value )
{
    return CypherMemory_Kilobytes( value ) * 1024u;
}
    
constexpr inline common::usize CypherMemory_Gigabytes( const common::usize value )
{
    return CypherMemory_Megabytes( value ) * 1024u;
}

/*
================
Arena Constants
================
*/
constexpr common::usize CYPHER_MEMORY_DEFAULT_ALIGNMENT = alignof( common::usize );    

constexpr common::usize CYPHER_MEMORY_KIB               = 1024u;
constexpr common::usize CYPHER_MEMORY_MIB               = 1024u * CYPHER_MEMORY_KIB;
constexpr common::usize CYPHER_MEMORY_GIB               = 1024u * CYPHER_MEMORY_MIB;

constexpr common::u32 CYPHER_MEMORY_ARENA_FLAG_NONE                  = 0u;               // 0000
constexpr common::u32 CYPHER_MEMORY_ARENA_FLAG_ZERO_ON_ALLOC         = 1u << 0u;         // 0001
constexpr common::u32 CYPHER_MEMORY_ARENA_FLAG_CLEAR_ON_RESET        = 1u << 1u;         // 0010
constexpr common::u32 CYPHER_MEMORY_ARENA_FLAG_CLEAR_ON_SHUTDOWN     = 1u << 2u;         // 0100
constexpr common::u32 CYPHER_MEMORY_ARENA_FLAG_GROW_COMMIT_ON_ALLOC  = 1u << 3u;         // 1000
constexpr common::u32 CYPHER_MEMORY_ARENA_FLAG_DECOMMIT_ON_RESET     = 1u << 4u;         // 1 0000

constexpr common::usize CYPHER_MEMORY_ARENA_ALLOCATION_TRACE_COUNT = 64u;

/*
================
Arena Helpers
================
*/
constexpr inline bool CypherMemory_IsPowerOfTwo( const common::usize value )
{   
    // Quick fast O(1) formula for checking if the binary value is power of two.
    return ( ( value > 0 ) && ( value & ( value - 1 ) ) == 0 );
}

constexpr inline bool CypherMemory_AddSizeChecked( const common::usize a, const common::usize b, common::usize &out_value )
{
    const common::usize max_value = std::numeric_limits<common::usize>::max();

    if ( a > max_value - b ) {
        return false;
    }

    out_value = a + b;
    return true;
}

constexpr inline bool CypherMemory_MulSizeChecked( const common::usize a, const common::usize b, common::usize &out_value )
{
    const common::usize max_value = std::numeric_limits<common::usize>::max();

    if ( a != 0u && b > max_value / a ) {
        return false;
    }

    out_value = a * b;
    return true;
}

constexpr inline bool CypherMemory_AlignForwardChecked( const common::usize value, const common::usize alignment, common::usize &out_value )
{
    if ( !CypherMemory_IsPowerOfTwo( alignment ) ) {
        return false;
    }

    const common::usize mask = alignment - 1u;

    if ( value > std::numeric_limits<common::usize>::max() - mask ) {
        return false;
    }

    out_value = ( value + mask ) & ~mask;
    return true;
}

constexpr inline common::usize CypherMemory_AlignForward( common::usize value, common::usize alignment ) 
{
    assert( CypherMemory_IsPowerOfTwo( alignment ) );

    common::usize result = value;
    const bool aligned = CypherMemory_AlignForwardChecked( value, alignment, result );
    assert( aligned );

    return result;
}

enum class arena_backing_t : common::u8 {
    ARENA_HEAP = 0,
    ARENA_EXTERNAL_BUFFER,
    ARENA_VIRTUAL_MEMORY  
};

/*
================
Arena Description

Creation request for an arena that owns its backing memory.
================
*/  
struct arena_desc_t {
    const char *name{ nullptr };
    
    common::usize capacity{ 0u };
    common::usize initial_commit{ 0u };
    void *external_buffer{ nullptr };
    
    common::u32 flags{ CYPHER_MEMORY_ARENA_FLAG_NONE };
    arena_backing_t backing{ arena_backing_t::ARENA_HEAP };
};

/*
================
Arena Marker

Saved arena offset. Rewinding to this marker releases everything allocated
after this point.
================
*/
struct arena_marker_t {
    common::usize used{ 0u };
};

/*
================
Arena Allocation Trace

Small debug record for recent arena allocation callsites.
================
*/
struct arena_allocation_trace_t {
    const char *file{ nullptr };
    const char *function{ nullptr };
    common::i32 line{ 0 };

    void *ptr{ nullptr };
    common::usize size{ 0u };
    common::usize alignment{ 0u };
    common::usize used_after{ 0u };

    common::u64 allocation_index{ 0u };
    error_code_t error{ error_code_t::OK };
    bool failed{ false };
};

/*
================
Arena Stats

Snapshot of arena memory usage.
================
*/
struct arena_stats_t {
    const char *name{ nullptr };
    
    common::usize capacity{ 0u };
    common::usize used{ 0u };
    common::usize remaining{ 0u };
    common::usize peak_used{ 0u };

    common::u64 allocation_count{ 0u };
    common::u64 failed_allocation_count{ 0u };

    common::usize committed{ 0u };
    common::usize initial_commit{ 0u };
};

/*
================
Arena

Linear allocator state. The arena owns or references one contiguous memory
block and serves allocations by moving the used offset forward.
================
*/
struct arena_t {
    const char *name{ nullptr };        // debugging/logging
    
    common::byte *base{ nullptr };      // pointer to the beginning of the arena memory block
    
    common::usize capacity{ 0u };       // max bytes allocation size
    common::usize used{ 0u };           // how much is used by this specific arena in arena.
    common::usize peak_used{ 0u };      // peak usage in frames for debugging.
    
    common::usize committed{ 0u };
    common::usize initial_commit{ 0u };
    common::usize page_size{ 0u };
    
    common::u64 allocation_count{ 0u }; // how many allocations happened for this, like usage
    common::u64 failed_allocation_count{ 0u };
    
    common::u32 flags{ CYPHER_MEMORY_ARENA_FLAG_NONE };
    
    error_code_t last_error{ error_code_t::OK };

    arena_allocation_trace_t allocation_traces[CYPHER_MEMORY_ARENA_ALLOCATION_TRACE_COUNT]{};
    common::usize allocation_trace_index{ 0u };
    common::usize allocation_trace_count{ 0u };
    
    arena_backing_t backing{ arena_backing_t::ARENA_HEAP };
    
    bool initialized{ false };          // did we init this arena or not
    bool owns_memory{ false };          // is it external or internal, arenas memory or not
};

/*
================
Arena Functions

List of functions necessary to use for creating arena memory layouts.
================
*/

error_code_t CypherMemory_ArenaInit( arena_t &arena, const arena_desc_t &arena_desc );

void CypherMemory_ArenaShutdown( arena_t &arena );

arena_stats_t CypherMemory_ArenaStats( const arena_t &arena );

void CypherMemory_ArenaResetCounters( arena_t &arena );

void CypherMemory_ArenaReset( arena_t &arena );
    
void *CypherMemory_ArenaAlloc( arena_t &arena, common::usize size, common::usize alignment = CYPHER_MEMORY_DEFAULT_ALIGNMENT );

void *CypherMemory_ArenaAllocDebug( arena_t &arena,
                                    common::usize size,
                                    common::usize alignment,
                                    const char *file,
                                    const char *function,
                                    common::i32 line );
    
void *CypherMemory_ArenaAllocZero( arena_t &arena, common::usize size, common::usize alignment = CYPHER_MEMORY_DEFAULT_ALIGNMENT );

void *CypherMemory_ArenaAllocZeroDebug( arena_t &arena,
                                        common::usize size,
                                        common::usize alignment,
                                        const char *file,
                                        const char *function,
                                        common::i32 line );

arena_marker_t CypherMemory_ArenaGetMarker( const arena_t &arena );

error_code_t CypherMemory_ArenaRewind( arena_t &arena, arena_marker_t marker );

bool CypherMemory_ArenaContains( const arena_t &arena, const void *ptr );

error_code_t CypherMemory_ArenaLastError( const arena_t &arena );

bool CypherMemory_ArenaIsInitialized( const arena_t &arena );

common::usize CypherMemory_ArenaUsed( const arena_t &arena );

common::f32 CypherMemory_ArenaUsageRatio( const arena_t &arena );

common::usize CypherMemory_ArenaCapacity( const arena_t &arena );

common::usize CypherMemory_ArenaRemaining( const arena_t &arena );

const arena_allocation_trace_t *CypherMemory_ArenaAllocationTraces( const arena_t &arena, common::usize &out_count );
        
/*
================
Arena Helper Functions

List of necessary and helpful functions
================
*/

template <typename T>
T *CypherMemory_ArenaAllocType( arena_t &arena ) {
    return static_cast<T *>(
            CypherMemory_ArenaAlloc(
            arena,
            sizeof( T ),
            alignof( T ) ) );
}

template <typename T>
T *CypherMemory_ArenaAllocTypeDebug( arena_t &arena, const char *file, const char *function, common::i32 line )
{
    return static_cast<T *>(
            CypherMemory_ArenaAllocDebug(
            arena,
            sizeof( T ),
            alignof( T ),
            file,
            function,
            line ) );
}

template <typename T>
T *CypherMemory_ArenaAllocArray( arena_t &arena, const common::usize count )
{
    common::usize size = 0u;
    if ( !CypherMemory_MulSizeChecked( sizeof( T ), count, size ) ) {
        arena.last_error = error_code_t::ERR_INTEGER_OVERFLOW;
        ++arena.failed_allocation_count;
        return nullptr;
    }

    return static_cast<T *>(
            CypherMemory_ArenaAlloc(
            arena,
            size,
            alignof( T ) ) );   
}

template <typename T>
T *CypherMemory_ArenaAllocArrayDebug( arena_t &arena, const common::usize count, const char *file, const char *function, common::i32 line )
{
    common::usize size = 0u;
    if ( !CypherMemory_MulSizeChecked( sizeof( T ), count, size ) ) {
        arena.last_error = error_code_t::ERR_INTEGER_OVERFLOW;
        ++arena.failed_allocation_count;
        return nullptr;
    }

    return static_cast<T *>(
            CypherMemory_ArenaAllocDebug(
            arena,
            size,
            alignof( T ),
            file,
            function,
            line ) );
}

template <typename T>
T *CypherMemory_ArenaAllocArrayZero( arena_t &arena, const common::usize count )
{
    common::usize size = 0u;
    if ( !CypherMemory_MulSizeChecked( sizeof( T ), count, size ) ) {
        arena.last_error = error_code_t::ERR_INTEGER_OVERFLOW;
        ++arena.failed_allocation_count;
        return nullptr;
    }

    return static_cast<T *>(
            CypherMemory_ArenaAllocZero(
            arena,
            size,
            alignof( T ) ) );   
}

template <typename T>
T *CypherMemory_ArenaAllocArrayZeroDebug( arena_t &arena, const common::usize count, const char *file, const char *function, common::i32 line )
{
    common::usize size = 0u;
    if ( !CypherMemory_MulSizeChecked( sizeof( T ), count, size ) ) {
        arena.last_error = error_code_t::ERR_INTEGER_OVERFLOW;
        ++arena.failed_allocation_count;
        return nullptr;
    }

    return static_cast<T *>(
            CypherMemory_ArenaAllocZeroDebug(
            arena,
            size,
            alignof( T ),
            file,
            function,
            line ) );
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

template <typename T>
T *CypherMemory_ArenaAllocTypeZeroDebug( arena_t &arena, const char *file, const char *function, common::i32 line )
{
        return static_cast<T *>(
            CypherMemory_ArenaAllocZeroDebug(
            arena,
            sizeof( T ),
            alignof( T ),
            file,
            function,
            line ) );
}

}       // namespace cypher::engine::memory

#define CYPHER_MEMORY_ARENA_ALLOC( ARENA, SIZE, ALIGNMENT ) \
    ::cypher::engine::memory::CypherMemory_ArenaAllocDebug( ( ARENA ), ( SIZE ), ( ALIGNMENT ), __FILE__, __func__, __LINE__ )

#define CYPHER_MEMORY_ARENA_ALLOC_ZERO( ARENA, SIZE, ALIGNMENT ) \
    ::cypher::engine::memory::CypherMemory_ArenaAllocZeroDebug( ( ARENA ), ( SIZE ), ( ALIGNMENT ), __FILE__, __func__, __LINE__ )

#define CYPHER_MEMORY_ARENA_ALLOC_TYPE( ARENA, TYPE ) \
    ::cypher::engine::memory::CypherMemory_ArenaAllocTypeDebug<TYPE>( ( ARENA ), __FILE__, __func__, __LINE__ )

#define CYPHER_MEMORY_ARENA_ALLOC_TYPE_ZERO( ARENA, TYPE ) \
    ::cypher::engine::memory::CypherMemory_ArenaAllocTypeZeroDebug<TYPE>( ( ARENA ), __FILE__, __func__, __LINE__ )

#define CYPHER_MEMORY_ARENA_ALLOC_ARRAY( ARENA, TYPE, COUNT ) \
    ::cypher::engine::memory::CypherMemory_ArenaAllocArrayDebug<TYPE>( ( ARENA ), ( COUNT ), __FILE__, __func__, __LINE__ )

#define CYPHER_MEMORY_ARENA_ALLOC_ARRAY_ZERO( ARENA, TYPE, COUNT ) \
    ::cypher::engine::memory::CypherMemory_ArenaAllocArrayZeroDebug<TYPE>( ( ARENA ), ( COUNT ), __FILE__, __func__, __LINE__ )

#pragma once

#include "CypherEngine/CypherCommon/CypherCommon.h"
#include "CypherEngine/CypherMemory/CypherMemory_Error.h"
#include <cassert>

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

constexpr inline common::usize CypherMemory_AlignForward( common::usize value, common::usize alignment ) 
{
    assert( CypherMemory_IsPowerOfTwo( alignment ) );
    
    // write the code here now for aligning forward in the arena memory block. 
    return ( value + alignment - 1u ) & ~( alignment - 1u );
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
    
    common::u8 *base{ nullptr };        // offset to the beginning of raw byte for taht alloc
    
    common::usize capacity{ 0u };       // max bytes allocation size
    common::usize used{ 0u };           // how much is used by this specific arena in arena.
    common::usize peak_used{ 0u };      // peak usage in frames for debugging.
    
    common::usize committed{ 0u };
    common::usize page_size{ 0u };
    
    common::u64 allocation_count{ 0u }; // how many allocations happened for this, like usage
    common::u64 failed_allocation_count{ 0u };
    
    common::u32 flags{ CYPHER_MEMORY_ARENA_FLAG_NONE };
    
    error_code_t last_error{ error_code_t::OK };
    
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


    
}       // namespace cypher::engine::memory


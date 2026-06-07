#pragma once

#include "CypherEngine/CypherCommon/CypherCommon.h"

namespace cypher::engine::memory
{
    
constexpr common::usize CYPHER_MEMORY_DEFAULT_ALIGNMENT = alignof( common::usize );    


constexpr common::usize CYPHER_MEMORY_KIB               = 1024u;
constexpr common::usize CYPHER_MEMORY_MIB               = 1024u * CYPHER_MEMORY_KIB;
constexpr common::usize CYPHER_MEMORY_GIB               = 1024u * CYPHER_MEMORY_MIB;

constexpr common::u32 CYPHER_MEMORY_ARENA_ZERO_ON_ALLOC = 1u << 0u;     // 0001
constexpr common::u32 CYPHER_MEMORY_ARENA_CLEAR_ON_RESET = 1u << 1u;    // 0010

struct arena_desc_t {
    const char *name{ nullptr };
    
    common::usize capacity{ 0u };
    common::u32 flags{ 0u };
};

struct arena_market_t {
    common::usize used{ 0u };
};

struct arena_t {
    const char *name{ nullptr };        // debugging/logging
    
    common::u8 *base{ nullptr };        // offset to the beginning of raw byte for taht alloc
    
    common::usize capacity{ 0u };       // max bytes allocation size
    common::usize used{ 0u };           // how much is used by this specific arena in arena.
    common::usize peak_used{ 0u };      // peak usage in frames for debugging.
    
    common::u64 allocation_count{ 0u }; // how many allocations happened for this, like usage
    
    common::f32 flags{};
    bool initialized{ false };          // did we init this arena or not
    bool owns_memory{ false };          // is it external or internal, arenas memory or not
};













    
    
    
    
    
}       // namespace cypher::engine::memory


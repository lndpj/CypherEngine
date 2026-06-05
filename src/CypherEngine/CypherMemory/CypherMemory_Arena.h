#pragma once

#include "CypherEngine/CypherCommon/CypherCommon.h"

namespace cypher::engine::memory
{

constexpr common::usize CYPHER_MEMORY_KIB       = 1024u;
constexpr common::usize CYPHER_MEMORY_MIB       = 1024u * CYPHER_MEMORY_KIB;
constexpr common::usize CYPHER_MEMORY_GIB       = 1024u * CYPHER_MEMORY_MIB;

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


/*======================================================================
   File: CypherMemory_Arena.cpp
   Project: CYPHER
   Author: ksiric <email@example.com>
   Created: 2026-06-07 12:32:50
   Last Modified by: ksiric
   Last Modified: 2026-06-07 12:54:49
   ---------------------------------------------------------------------
   Description:
       
   ---------------------------------------------------------------------
   License: 
   Company: 
   Version: 0.1.0
 ======================================================================
                                                                       */

#include "CypherEngine/CypherMemory/CypherMemory_Arena.h"

#include <cstdlib>          // for malloc func
#include <new>              // for operator new

namespace cypher::engine::memory
{

error_code_t CypherMemory_ArenaInit( arena_t &arena, const arena_desc_t &arena_desc )
{
    if ( arena.initialized ) {
        return error_code_t::ERR_ALREADY_INITIALIZED;
    }
    if ( arena_desc.capacity == 0u ) {
        return error_code_t::ERR_INVALID_CAPACITY;
    }
    
    // allocate new physical memory space -> for virtual we reserve and then commit in pages.
    void *memory = ::operator new( arena_desc.capacity, std::nothrow );
    if ( memory == nullptr ) {
        arena.last_error = error_code_t::ERR_MEMORY_ALLOCATION;
        return error_code_t::ERR_MEMORY_ALLOCATION;
    }
    
    return error_code_t::OK;
}
    
    
    
    
    
    
    
    
    
    
    
}       // namespace cypher::engine::memory

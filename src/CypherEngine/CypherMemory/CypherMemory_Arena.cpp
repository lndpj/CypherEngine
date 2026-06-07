/*======================================================================
   File: CypherMemory_Arena.cpp
   Project: CYPHER
   Author: ksiric <email@example.com>
   Created: 2026-06-07 12:32:50
   Last Modified by: ksiric
   Last Modified: 2026-06-07 12:42:04
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
    
    void *memory = ::operator new( arena_desc.capacity, std::nothrow );
    if ( memory == nullptr ) {
        return error_code_t::ERR_MEMORY_ALLOCATION;
    }
    
    
       
    
       
    
    
}
    
    
    
    
    
    
    
    
    
    
    
}       // namespace cypher::engine::memory

#ifndef CYPHER_COMMON_TIER0_THREAD_H
#define CYPHER_COMMON_TIER0_THREAD_H
#pragma once

/*
================
CypherCommon Thread

Minimal thread helpers. This is not the job system.
================
*/

#include "CypherCommon_BaseTypes.h"

#include <atomic>
#include <chrono>
#include <functional>
#include <thread>

namespace cypher::common
{

template <typename type_t>
using atomic_t = std::atomic<type_t>;

using memory_order_t = std::memory_order;

inline void ThreadYield()
{
    std::this_thread::yield();
}

inline void ThreadSleepMs( u32 milliseconds )
{
    std::this_thread::sleep_for( std::chrono::milliseconds( milliseconds ) );
}

inline u64 GetCurrentThreadIdHash()
{
    return static_cast<u64>( std::hash<std::thread::id>{}( std::this_thread::get_id() ) );
}

inline u32 GetLogicalThreadCount()
{
    const u32 thread_count = std::thread::hardware_concurrency();
    return thread_count != 0u ? thread_count : 1u;
}

} // namespace cypher::common

#endif // CYPHER_COMMON_TIER0_THREAD_H

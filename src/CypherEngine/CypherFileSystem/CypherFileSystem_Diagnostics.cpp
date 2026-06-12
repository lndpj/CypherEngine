#include "CypherEngine/CypherFileSystem/CypherFileSystem_Runtime.h"
#include "CypherEngine/CypherLog/CypherLog.h"

namespace cypher::engine::fs
{

fs_error_t CypherFileSystem_GetStats( stats_t &out_stats )
{
    std::lock_guard<std::recursive_mutex> lock( CypherFileSystem_RuntimeMutex() );
    const runtime_state_t &state = CypherFileSystem_RuntimeState();
    if ( !state.initialized ) {
        out_stats = {};
        return fs_error_t::ERR_NOT_INIT;
    }

    out_stats = state.stats;
    return fs_error_t::OK;
}

fs_error_t CypherFileSystem_ResetStats()
{
    std::lock_guard<std::recursive_mutex> lock( CypherFileSystem_RuntimeMutex() );
    runtime_state_t &state = CypherFileSystem_RuntimeState();
    if ( !state.initialized ) {
        return fs_error_t::ERR_NOT_INIT;
    }

    state.stats = {};
    return fs_error_t::OK;
}

fs_error_t CypherFileSystem_DumpMounts()
{
    std::lock_guard<std::recursive_mutex> lock( CypherFileSystem_RuntimeMutex() );
    const runtime_state_t &state = CypherFileSystem_RuntimeState();
    if ( !state.initialized ) {
        return fs_error_t::ERR_NOT_INIT;
    }

    LOG_INFO( log::channel_t::FS, "filesystem mounts: count=%u.", state.mount_count );
    for ( common::u32 i = 0u; i < state.mount_count; ++i ) {
        const mount_t &mount = state.mounts[i];
        LOG_INFO(
            log::channel_t::FS,
            "filesystem mount[%u]: handle=%u type=%u virtual='%s' physical='%s' flags=0x%x priority=%u.",
            i,
            mount.handle,
            static_cast<common::u32>( mount.type ),
            mount.virtual_root[0] != '\0' ? mount.virtual_root : "<root>",
            mount.physical_root,
            mount.flags,
            mount.priority );
    }

    return fs_error_t::OK;
}

fs_error_t CypherFileSystem_DumpStats()
{
    std::lock_guard<std::recursive_mutex> lock( CypherFileSystem_RuntimeMutex() );
    const runtime_state_t &state = CypherFileSystem_RuntimeState();
    if ( !state.initialized ) {
        return fs_error_t::ERR_NOT_INIT;
    }

    const stats_t &stats = state.stats;
    LOG_INFO(
        log::channel_t::FS,
        "filesystem stats: open=%llu close=%llu read=%llu write=%llu bytes_read=%llu bytes_written=%llu failed_lookup=%llu.",
        static_cast<unsigned long long>( stats.open_count ),
        static_cast<unsigned long long>( stats.close_count ),
        static_cast<unsigned long long>( stats.read_count ),
        static_cast<unsigned long long>( stats.write_count ),
        static_cast<unsigned long long>( stats.bytes_read ),
        static_cast<unsigned long long>( stats.bytes_written ),
        static_cast<unsigned long long>( stats.failed_lookup_count ) );

    return fs_error_t::OK;
}

}       // namespace cypher::engine::fs

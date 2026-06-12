#include "CypherEngine/CypherFileSystem/CypherFileSystem_Runtime.h"

namespace cypher::engine::fs
{

fs_error_t CypherFileSystem_WatchPath(
    const char *virtual_path,
    common::u32 flags,
    watch_handle_t &out_watch )
{
    std::lock_guard<std::recursive_mutex> lock( CypherFileSystem_RuntimeMutex() );
    ( void )flags;
    out_watch = CYPHER_FILESYSTEM_INVALID_WATCH;

    if ( !CypherFileSystem_RuntimeState().initialized ) {
        return fs_error_t::ERR_NOT_INIT;
    }
    if ( virtual_path == nullptr || virtual_path[0] == '\0' ) {
        return fs_error_t::ERR_INVALID_PATH;
    }

    return fs_error_t::ERR_NOT_IMPLEMENTED;
}

fs_error_t CypherFileSystem_UnwatchPath( watch_handle_t watch )
{
    std::lock_guard<std::recursive_mutex> lock( CypherFileSystem_RuntimeMutex() );
    if ( !CypherFileSystem_RuntimeState().initialized ) {
        return fs_error_t::ERR_NOT_INIT;
    }
    if ( watch == CYPHER_FILESYSTEM_INVALID_WATCH ) {
        return fs_error_t::ERR_INVALID_HANDLE;
    }

    return fs_error_t::ERR_NOT_IMPLEMENTED;
}

fs_error_t CypherFileSystem_PollChanges(
    watch_event_t *events,
    common::u32 max_events,
    common::u32 &out_event_count )
{
    std::lock_guard<std::recursive_mutex> lock( CypherFileSystem_RuntimeMutex() );
    out_event_count = 0u;

    if ( !CypherFileSystem_RuntimeState().initialized ) {
        return fs_error_t::ERR_NOT_INIT;
    }
    if ( events == nullptr && max_events != 0u ) {
        return fs_error_t::ERR_INVALID_ARGUMENT;
    }

    return fs_error_t::OK;
}

}       // namespace cypher::engine::fs

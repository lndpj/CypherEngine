#include "CypherEngine/CypherFileSystem/CypherFileSystem_Runtime.h"

namespace cypher::engine::fs
{

fs_error_t CypherFileSystem_ReadAsync(
    const char *virtual_path,
    void *buffer,
    common::u64 bytes_to_read,
    async_request_t &out_request )
{
    std::lock_guard<std::recursive_mutex> lock( CypherFileSystem_RuntimeMutex() );
    out_request = CYPHER_FILESYSTEM_INVALID_ASYNC_REQUEST;

    if ( !CypherFileSystem_RuntimeState().initialized ) {
        return fs_error_t::ERR_NOT_INIT;
    }
    if ( virtual_path == nullptr || virtual_path[0] == '\0' ) {
        return fs_error_t::ERR_INVALID_PATH;
    }
    if ( bytes_to_read != 0u && buffer == nullptr ) {
        return fs_error_t::ERR_INVALID_ARGUMENT;
    }

    return fs_error_t::ERR_NOT_IMPLEMENTED;
}

fs_error_t CypherFileSystem_WriteAsync(
    const char *virtual_path,
    const void *buffer,
    common::u64 bytes_to_write,
    async_request_t &out_request )
{
    std::lock_guard<std::recursive_mutex> lock( CypherFileSystem_RuntimeMutex() );
    out_request = CYPHER_FILESYSTEM_INVALID_ASYNC_REQUEST;

    if ( !CypherFileSystem_RuntimeState().initialized ) {
        return fs_error_t::ERR_NOT_INIT;
    }
    if ( virtual_path == nullptr || virtual_path[0] == '\0' ) {
        return fs_error_t::ERR_INVALID_PATH;
    }
    if ( bytes_to_write != 0u && buffer == nullptr ) {
        return fs_error_t::ERR_INVALID_ARGUMENT;
    }

    return fs_error_t::ERR_NOT_IMPLEMENTED;
}

fs_error_t CypherFileSystem_PollAsync( async_request_t request, async_result_t &out_result )
{
    std::lock_guard<std::recursive_mutex> lock( CypherFileSystem_RuntimeMutex() );
    out_result = {};

    if ( !CypherFileSystem_RuntimeState().initialized ) {
        return fs_error_t::ERR_NOT_INIT;
    }
    if ( request == CYPHER_FILESYSTEM_INVALID_ASYNC_REQUEST ) {
        out_result.error = fs_error_t::ERR_INVALID_HANDLE;
        return fs_error_t::ERR_INVALID_HANDLE;
    }

    out_result.error = fs_error_t::ERR_NOT_IMPLEMENTED;
    return fs_error_t::ERR_NOT_IMPLEMENTED;
}

fs_error_t CypherFileSystem_WaitAsync( async_request_t request, async_result_t &out_result )
{
    return CypherFileSystem_PollAsync( request, out_result );
}

fs_error_t CypherFileSystem_CancelAsync( async_request_t request )
{
    std::lock_guard<std::recursive_mutex> lock( CypherFileSystem_RuntimeMutex() );
    if ( !CypherFileSystem_RuntimeState().initialized ) {
        return fs_error_t::ERR_NOT_INIT;
    }
    if ( request == CYPHER_FILESYSTEM_INVALID_ASYNC_REQUEST ) {
        return fs_error_t::ERR_INVALID_HANDLE;
    }

    return fs_error_t::ERR_NOT_IMPLEMENTED;
}

}       // namespace cypher::engine::fs

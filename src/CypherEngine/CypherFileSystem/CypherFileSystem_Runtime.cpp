#include "CypherEngine/CypherFileSystem/CypherFileSystem_Runtime.h"

namespace cypher::engine::fs
{

namespace {

runtime_state_t g_fs_runtime_state{};

}       // namespace

runtime_state_t &CypherFileSystem_RuntimeState()
{
    return g_fs_runtime_state;
}

bool CypherFileSystem_HasWritePath()
{
    const runtime_state_t &state = CypherFileSystem_RuntimeState();
    return state.initialized && state.write_path[0] != '\0';
}

fs_error_t CypherFileSystem_BuildWritePath( const char *virtual_path, char *out_path, common::u32 out_path_size )
{
    runtime_state_t &state = CypherFileSystem_RuntimeState();

    if ( !state.initialized ) {
        return fs_error_t::ERR_NOT_INIT;
    }
    if ( state.write_path[0] == '\0' ) {
        return fs_error_t::ERR_WRITE_PATH_NOT_SET;
    }
    if ( out_path == nullptr || out_path_size == 0u ) {
        return fs_error_t::ERR_INVALID_ARGUMENT;
    }

    char normalized_path[CYPHER_FILESYSTEM_MAX_PATH_LENGTH]{};
    const fs_error_t normalize_result = CypherFileSystem_NormalizeVirtualPath( virtual_path, normalized_path, sizeof( normalized_path ) );
    if ( normalize_result != fs_error_t::OK ) {
        out_path[0] = '\0';
        return normalize_result;
    }

    return CypherFileSystem_BuildPhysicalPath( state.write_path, normalized_path, out_path, out_path_size );
}

}       // namespace cypher::engine::fs

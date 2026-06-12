#pragma once

#include "CypherEngine/CypherFileSystem/CypherFileSystem.h"

namespace cypher::engine::fs
{

/*
================
Filesystem Runtime

Shared state and helpers used only by filesystem implementation files.
================
*/
struct runtime_state_t {
    bool initialized{ false };
    mount_t mounts[CYPHER_FILESYSTEM_MAX_MOUNTS]{};
    common::u32 mount_count{ 0u };
    char write_path[CYPHER_FILESYSTEM_MAX_PATH_LENGTH]{};
};

runtime_state_t &CypherFileSystem_RuntimeState();

bool CypherFileSystem_HasWritePath();

fs_error_t CypherFileSystem_BuildWritePath(
    const char *virtual_path,
    char *out_path,
    common::u32 out_path_size );

}       // namespace cypher::engine::fs

#pragma once

#include "CypherEngine/CypherFileSystem/CypherFileSystem_Error.h"
#include "CypherEngine/CypherFileSystem/CypherFileSystem_Types.h"

namespace cypher::engine::fs
{

/*
================
Filesystem API

Mount-based virtual file system used by configs, shaders and future assets.
================
*/
cypher_filesystem_error_code_t CypherFileSystem_Init();

cypher_filesystem_error_code_t CypherFileSystem_Shutdown();

cypher_filesystem_error_code_t CypherFileSystem_MountDirectory(
    const char *virtual_root,
    const char *physical_path,
    common::u32 flags,
    common::u32 priority );

cypher_filesystem_error_code_t CypherFileSystem_UnmountDirectory( const char *virtual_root );

common::u32 CypherFileSystem_MountCount();

cypher_filesystem_error_code_t CypherFileSystem_SetWritePath( const char *physical_path );

const char *CypherFileSystem_GetWritePath();

cypher_filesystem_error_code_t CypherFileSystem_Open(
    const char *virtual_path,
    cypher_filesystem_open_mode_t mode,
    cypher_filesystem_file_t &file );

cypher_filesystem_error_code_t CypherFileSystem_Close( cypher_filesystem_file_t &file );

cypher_filesystem_error_code_t CypherFileSystem_Read(
    cypher_filesystem_file_t &file,
    void *buffer,
    common::u64 bytes_to_read,
    common::u64 &bytes_read_out );

cypher_filesystem_error_code_t CypherFileSystem_Write(
    cypher_filesystem_file_t &file,
    const void *buffer,
    common::u64 bytes_to_write,
    common::u64 &bytes_written_out );

cypher_filesystem_error_code_t CypherFileSystem_Seek(
    cypher_filesystem_file_t &file,
    common::i64 offset,
    cypher_filesystem_seek_origin_t origin );

cypher_filesystem_error_code_t CypherFileSystem_Tell(
    cypher_filesystem_file_t &file,
    common::u64 &out_position );

cypher_filesystem_error_code_t CypherFileSystem_ReadEntireFile(
    const char *virtual_path,
    void *buffer,
    common::u64 bytes_to_read,
    common::u64 &bytes_read_out );

cypher_filesystem_error_code_t CypherFileSystem_GetFileInfo(
    const char *virtual_path,
    cypher_filesystem_file_info_t &out_info );

cypher_filesystem_error_code_t CypherFileSystem_ResolvePath(
    const char *virtual_path,
    char *out_resolved_path,
    common::u32 out_resolved_path_size );

bool CypherFileSystem_Exists( const char *virtual_path );

bool CypherFileSystem_IsInitialized();

}

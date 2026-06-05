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
error_code_t CypherFileSystem_Init();

error_code_t CypherFileSystem_Shutdown();

error_code_t CypherFileSystem_MountDirectory(
    const char *virtual_root,
    const char *physical_path,
    common::u32 flags,
    common::u32 priority );

error_code_t CypherFileSystem_UnmountDirectory( const char *virtual_root );

common::u32 CypherFileSystem_MountCount();

error_code_t CypherFileSystem_SetWritePath( const char *physical_path );

const char *CypherFileSystem_GetWritePath();

error_code_t CypherFileSystem_Open(
    const char *virtual_path,
    open_mode_t mode,
    file_t &file );

error_code_t CypherFileSystem_Close( file_t &file );

error_code_t CypherFileSystem_Read(
    file_t &file,
    void *buffer,
    common::u64 bytes_to_read,
    common::u64 &bytes_read_out );

error_code_t CypherFileSystem_Write(
    file_t &file,
    const void *buffer,
    common::u64 bytes_to_write,
    common::u64 &bytes_written_out );

error_code_t CypherFileSystem_Seek(
    file_t &file,
    common::i64 offset,
    seek_origin_t origin );

error_code_t CypherFileSystem_Tell(
    file_t &file,
    common::u64 &out_position );

error_code_t CypherFileSystem_ReadEntireFile(
    const char *virtual_path,
    void *buffer,
    common::u64 bytes_to_read,
    common::u64 &bytes_read_out );

error_code_t CypherFileSystem_GetFileInfo(
    const char *virtual_path,
    file_info_t &out_info );

error_code_t CypherFileSystem_ResolvePath(
    const char *virtual_path,
    char *out_resolved_path,
    common::u32 out_resolved_path_size );

bool CypherFileSystem_Exists( const char *virtual_path );

bool CypherFileSystem_IsInitialized();

}

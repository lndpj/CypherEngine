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
fs_error_t CypherFileSystem_Init();

fs_error_t CypherFileSystem_Shutdown();

fs_error_t CypherFileSystem_MountDirectory(
    const char *virtual_root,
    const char *physical_path,
    common::u32 flags,
    common::u32 priority );

fs_error_t CypherFileSystem_UnmountDirectory( const char *virtual_root );

common::u32 CypherFileSystem_MountCount();

fs_error_t CypherFileSystem_SetWritePath( const char *physical_path );

const char *CypherFileSystem_GetWritePath();

fs_error_t CypherFileSystem_Open(
    const char *virtual_path,
    open_mode_t mode,
    file_t &file );

fs_error_t CypherFileSystem_Close( file_t &file );

fs_error_t CypherFileSystem_Read(
    file_t &file,
    void *buffer,
    common::u64 bytes_to_read,
    common::u64 &bytes_read_out );

fs_error_t CypherFileSystem_Write(
    file_t &file,
    const void *buffer,
    common::u64 bytes_to_write,
    common::u64 &bytes_written_out );

fs_error_t CypherFileSystem_Seek(
    file_t &file,
    common::i64 offset,
    seek_origin_t origin );

fs_error_t CypherFileSystem_Tell(
    file_t &file,
    common::u64 &out_position );

fs_error_t CypherFileSystem_ReadEntireFile(
    const char *virtual_path,
    void *buffer,
    common::u64 bytes_to_read,
    common::u64 &bytes_read_out );

fs_error_t CypherFileSystem_GetFileInfo(
    const char *virtual_path,
    file_info_t &out_info );

fs_error_t CypherFileSystem_ResolvePath(
    const char *virtual_path,
    char *out_resolved_path,
    common::u32 out_resolved_path_size );

fs_error_t CypherFileSystem_NormalizeVirtualPath( const char *virtual_path, char *out_path, common::u32 out_path_size );

fs_error_t CypherFileSystem_NormalizeVirtualRoot( const char *virtual_root, char *out_root, common::u32 out_size );

fs_error_t CypherFileSystem_BuildPhysicalPath( const char *physical_root, const char *relative_path, char *out_path, common::u32 out_path_size );

bool CypherFileSystem_VirtualPathStartsWithRoot( const char *virtual_path, const char *virtual_root, const char **out_relative_path );

bool CypherFileSystem_Exists( const char *virtual_path );

bool CypherFileSystem_IsInitialized();

}       // namespace cypher::engine::fs

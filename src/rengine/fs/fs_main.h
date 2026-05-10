#pragma once

#include "rengine/fs/fs_error.h"
#include "rengine/fs/fs_types.h"

namespace reap::rengine::fs
{

/*
================
Filesystem API

Mount-based virtual file system used by configs, shaders and future assets.
================
*/
fs_error_code_t FS_Init();

fs_error_code_t FS_Shutdown();

fs_error_code_t FS_MountDirectory(
    const char *virtual_root,
    const char *physical_path,
    rcommon::u32 flags,
    rcommon::u32 priority );

fs_error_code_t FS_UnmountDirectory( const char *virtual_root );

rcommon::u32 FS_MountCount();

fs_error_code_t FS_SetWritePath( const char *physical_path );

const char *FS_GetWritePath();

fs_error_code_t FS_Open(
    const char *virtual_path,
    fs_open_mode_t mode,
    fs_file_t &file );

fs_error_code_t FS_Close( fs_file_t &file );

fs_error_code_t FS_Read(
    fs_file_t &file,
    void *buffer,
    rcommon::u64 bytes_to_read,
    rcommon::u64 &bytes_read_out );

fs_error_code_t FS_Write(
    fs_file_t &file,
    const void *buffer,
    rcommon::u64 bytes_to_write,
    rcommon::u64 &bytes_written_out );

fs_error_code_t FS_Seek(
    fs_file_t &file,
    rcommon::i64 offset,
    fs_seek_origin_t origin );

fs_error_code_t FS_Tell(
    fs_file_t &file,
    rcommon::u64 &out_position );

fs_error_code_t FS_ReadEntireFile(
    const char *virtual_path,
    void *buffer,
    rcommon::u64 bytes_to_read,
    rcommon::u64 &bytes_read_out );

fs_error_code_t FS_GetFileInfo(
    const char *virtual_path,
    fs_file_info_t &out_info );

fs_error_code_t FS_ResolvePath(
    const char *virtual_path,
    char *out_resolved_path,
    rcommon::u32 out_resolved_path_size );

bool FS_Exists( const char *virtual_path );

bool FS_IsInitialized();

}

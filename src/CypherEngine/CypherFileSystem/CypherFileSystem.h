#ifndef CYPHER_ENGINE_FILESYSTEM_H
#define CYPHER_ENGINE_FILESYSTEM_H

#pragma once

#include "CypherEngine/CypherFileSystem/CypherFileSystem_Error.h"
#include "CypherEngine/CypherFileSystem/CypherFileSystem_Types.h"

namespace cypher::engine::fs
{

/*
================
Filesystem API

Mount-based virtual file system used by configs, shaders, assets, packages,
editor browsing, save data and future hot reload.
================
*/

/*
================
Lifecycle
================
*/
fs_error_t CypherFileSystem_Init();

fs_error_t CypherFileSystem_Shutdown();

bool CypherFileSystem_IsInitialized();

/*
================
Mounts

Reads search mounted content by priority. Higher priority wins; equal priority
keeps insertion order stable so overlays remain predictable.
================
*/
fs_error_t CypherFileSystem_MountDirectory(
    const char *virtual_root,
    const char *physical_path,
    common::u32 flags,
    common::u32 priority );

fs_error_t CypherFileSystem_MountDirectoryWithHandle(
    const char *virtual_root,
    const char *physical_path,
    common::u32 flags,
    common::u32 priority,
    mount_handle_t &out_handle );

fs_error_t CypherFileSystem_UnmountDirectory( const char *virtual_root );

fs_error_t CypherFileSystem_Unmount( mount_handle_t mount );

fs_error_t CypherFileSystem_MountPackage(
    const char *virtual_root,
    const char *package_path,
    common::u32 flags,
    common::u32 priority );

fs_error_t CypherFileSystem_UnmountPackage( const char *package_path );

common::u32 CypherFileSystem_MountCount();

fs_error_t CypherFileSystem_GetMountInfo(
    common::u32 mount_index,
    mount_info_t &out_info );

fs_error_t CypherFileSystem_GetMountInfoByHandle(
    mount_handle_t mount,
    mount_info_t &out_info );

/*
================
Path Policy

Virtual paths use forward slashes, are relative to virtual roots, reject '..',
reject absolute paths, reject drive letters and normalize asset paths to lower
case internally for cross-platform consistency.
================
*/
fs_error_t CypherFileSystem_NormalizeVirtualPath(
    const char *virtual_path,
    char *out_path,
    common::u32 out_path_size );

fs_error_t CypherFileSystem_NormalizeVirtualRoot(
    const char *virtual_root,
    char *out_root,
    common::u32 out_size );

bool CypherFileSystem_IsValidVirtualPath( const char *virtual_path );

bool CypherFileSystem_VirtualPathStartsWithRoot(
    const char *virtual_path,
    const char *virtual_root,
    const char **out_relative_path );

fs_error_t CypherFileSystem_BuildPhysicalPath(
    const char *physical_root,
    const char *relative_path,
    char *out_path,
    common::u32 out_path_size );

fs_error_t CypherFileSystem_ResolvePath(
    const char *virtual_path,
    char *out_resolved_path,
    common::u32 out_resolved_path_size );

fs_error_t CypherFileSystem_TraceResolve(
    const char *virtual_path,
    resolve_trace_t &out_trace );

fs_error_t CypherFileSystem_PathJoin(
    const char *left,
    const char *right,
    char *out_path,
    common::u32 out_path_size );

const char *CypherFileSystem_PathBasename( const char *virtual_path );

fs_error_t CypherFileSystem_PathDirname(
    const char *virtual_path,
    char *out_path,
    common::u32 out_path_size );

const char *CypherFileSystem_PathExtension( const char *virtual_path );

fs_error_t CypherFileSystem_PathWithoutExtension(
    const char *virtual_path,
    char *out_path,
    common::u32 out_path_size );

bool CypherFileSystem_PathHasExtension(
    const char *virtual_path,
    const char *extension );

/*
================
Write Root

Writes, deletes, renames and generated data go through the write path. They do
not modify arbitrary read mounts.
================
*/
fs_error_t CypherFileSystem_SetWritePath( const char *physical_path );

const char *CypherFileSystem_GetWritePath();

/*
================
File I/O
================
*/
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

fs_error_t CypherFileSystem_Flush( file_t &file );

fs_error_t CypherFileSystem_ReadEntireFile(
    const char *virtual_path,
    void *buffer,
    common::u64 bytes_to_read,
    common::u64 &bytes_read_out );

fs_error_t CypherFileSystem_WriteEntireFile(
    const char *virtual_path,
    const void *buffer,
    common::u64 bytes_to_write );

fs_error_t CypherFileSystem_AppendEntireFile(
    const char *virtual_path,
    const void *buffer,
    common::u64 bytes_to_write );

/*
================
Write-Side Management

These APIs operate under the write path. RemoveDirectory is intentionally
non-recursive; destructive recursive removal is a separate explicit API.
================
*/
fs_error_t CypherFileSystem_CreateDirectory( const char *virtual_path );

fs_error_t CypherFileSystem_DeleteFile( const char *virtual_path );

fs_error_t CypherFileSystem_RemoveDirectory( const char *virtual_path );

fs_error_t CypherFileSystem_RemoveDirectoryTree( const char *virtual_path );

fs_error_t CypherFileSystem_Rename(
    const char *from_virtual_path,
    const char *to_virtual_path );

fs_error_t CypherFileSystem_CopyFile(
    const char *from_virtual_path,
    const char *to_virtual_path );

/*
================
Query And Discovery
================
*/
bool CypherFileSystem_Exists( const char *virtual_path );

bool CypherFileSystem_FileExists( const char *virtual_path );

bool CypherFileSystem_DirectoryExists( const char *virtual_path );

fs_error_t CypherFileSystem_GetFileInfo(
    const char *virtual_path,
    file_info_t &out_info );

fs_error_t CypherFileSystem_ListDirectory(
    const char *virtual_path,
    directory_entry_t *entries,
    common::u32 max_entries,
    common::u32 &out_entry_count );

fs_error_t CypherFileSystem_FindFiles(
    const char *virtual_root,
    const char *pattern,
    common::u32 flags,
    directory_entry_t *entries,
    common::u32 max_entries,
    common::u32 &out_entry_count );

/*
================
Packages

Package APIs will back .pak/.zip/custom package mounts later. The public API is
declared now so engine code has a stable target surface.
================
*/
fs_error_t CypherFileSystem_GetPackageInfo(
    const char *package_path,
    package_info_t &out_info );

bool CypherFileSystem_PackageIsMounted( const char *package_path );

/*
================
Async And Streaming

The first implementation can be simple worker-thread file reads. Later this
becomes the streaming layer for textures, audio, levels and packages.
================
*/
fs_error_t CypherFileSystem_ReadAsync(
    const char *virtual_path,
    void *buffer,
    common::u64 bytes_to_read,
    async_request_t &out_request );

fs_error_t CypherFileSystem_WriteAsync(
    const char *virtual_path,
    const void *buffer,
    common::u64 bytes_to_write,
    async_request_t &out_request );

fs_error_t CypherFileSystem_PollAsync(
    async_request_t request,
    async_result_t &out_result );

fs_error_t CypherFileSystem_WaitAsync(
    async_request_t request,
    async_result_t &out_result );

fs_error_t CypherFileSystem_CancelAsync( async_request_t request );

/*
================
File Watching

Used by editor workflows and hot reload for shaders, configs and assets.
================
*/
fs_error_t CypherFileSystem_WatchPath(
    const char *virtual_path,
    common::u32 flags,
    watch_handle_t &out_watch );

fs_error_t CypherFileSystem_UnwatchPath( watch_handle_t watch );

fs_error_t CypherFileSystem_PollChanges(
    watch_event_t *events,
    common::u32 max_events,
    common::u32 &out_event_count );

/*
================
Diagnostics

Runtime visibility is mandatory for a professional filesystem: mount dumps,
lookup failures, bytes read/written and package/debug inspection.
================
*/
fs_error_t CypherFileSystem_GetStats( stats_t &out_stats );

fs_error_t CypherFileSystem_ResetStats();

fs_error_t CypherFileSystem_DumpMounts();

fs_error_t CypherFileSystem_DumpStats();

}       // namespace cypher::engine::fs

#endif // CYPHER_ENGINE_FILESYSTEM_H

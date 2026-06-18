#include "CypherEngine/CypherFileSystem/CypherFileSystem.h"
#include "CypherEngine/CypherFileSystem/CypherFileSystem_Runtime.h"
#include "CypherEngine/CypherSystem/CypherSystem_Platform.h"

#include <cstring>          // memcpy etc...
#include <chrono>           // for system_clock etc...
#include <string>           // for std::string etc...
#include <filesystem>       // for std::filesystem::exists and etc...
#include <system_error>     // for error_codes etc...

// @Windows Including and Checking
#if defined( CYPHER_PLATFORM_WINDOWS )
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#endif

namespace cypher::engine::fs
{

namespace {

#if defined( CYPHER_PLATFORM_WINDOWS )

struct windows_watch_t {
    bool used{ false };                                      // is this current watch in use or not.
    HANDLE directory_handle{ INVALID_HANDLE_VALUE };                        // Windows OS directory handle for kernel/user object
    HANDLE event_handle{ nullptr };                            // native Windows OS event handle reference for kernel/
                                                    // user object
    OVERLAPPED overlapped{};                          //
    unsigned char buffer[64u * 1024u]{};            // buffer into which the changes will be written
    bool read_pending{ false };                     // one currently active ReadDirectoryChangesW
    bool watch_file{ false };

    char watch_physical_path[CYPHER_FILESYSTEM_MAX_PATH_LENGTH]{};
    char watch_virtual_path[CYPHER_FILESYSTEM_MAX_PATH_LENGTH]{};
    char file_filter[CYPHER_FILESYSTEM_MAX_PATH_LENGTH]{};

    char pending_rename_old_virtual[CYPHER_FILESYSTEM_MAX_PATH_LENGTH]{};
    bool pending_rename{ false };
};

static windows_watch_t g_windows_watches[CYPHER_FILESYSTEM_MAX_WATCHES]{};

#endif

static fs_error_t ResolveWatchPhysicalPath( runtime_state_t &state,
                                            const char *virtual_path,
                                            char *out_normalized_path,
                                            common::u32 out_normalized_path_size,
                                            char *out_physical_path,
                                            common::u32 out_physical_path_size,
                                            bool &out_is_directory )
{
    out_is_directory = false;
    if ( virtual_path == nullptr || virtual_path[0] == '\0' ) {
        return fs_error_t::ERR_INVALID_PATH;
    }
    if ( out_normalized_path == nullptr || out_normalized_path_size == 0u || out_physical_path == nullptr || out_physical_path_size == 0u ) {
        return fs_error_t::ERR_INVALID_ARGUMENT;
    }
    out_normalized_path[0] = '\0';
    out_physical_path[0] = '\0';

    fs_error_t result = CypherFileSystem_NormalizeVirtualPath( virtual_path, out_normalized_path, out_normalized_path_size );
    if ( result != fs_error_t::OK ) {
        return result;
    }
    for ( common::u32 i = 0; i < state.mount_count; ++i ) {
        const mount_t &mount = state.mounts[i];
        if ( mount.type != mount_type_t::CYPHER_FILESYSTEM_DIRECTORY ) {
            continue;
        }
        const char *relative_path = nullptr;
        if ( !CypherFileSystem_VirtualPathStartsWithRoot( out_normalized_path, mount.virtual_root, &relative_path ) ) {
            continue;
        }
        if ( relative_path == nullptr ) {
            continue;
        }
        char possible_path[CYPHER_FILESYSTEM_MAX_PATH_LENGTH]{};
        result = CypherFileSystem_BuildPhysicalPath( mount.physical_root, relative_path, possible_path, sizeof( possible_path ) );
        if ( result != fs_error_t::OK ) {
            return result;
        }
        std::error_code ec{};
        if ( !std::filesystem::exists( possible_path, ec ) ) {
            if ( ec ) {
                return fs_error_t::ERR_IO_ERROR;
            }
            continue;
        }
        const bool is_directory = std::filesystem::is_directory( possible_path, ec );
        if ( ec ) {
            return fs_error_t::ERR_IO_ERROR;
        }
        const common::usize possible_path_len = std::strlen( possible_path );
        if ( possible_path_len + 1u > out_physical_path_size ) {
            return fs_error_t::ERR_BUFFER_TOO_SMALL;
        }
        std::memcpy( out_physical_path, possible_path, possible_path_len + 1u );
        out_is_directory = is_directory;
        return fs_error_t::OK;
    }
    return fs_error_t::ERR_PATH_NOT_FOUND;
}       // ResolveWatchPhysicalPath

static fs_error_t ValidateWatchFlags( common::u32 flags, bool is_directory )
{
    const common::u32 allowed_flags = CYPHER_FILESYSTEM_WATCH_FILE |
                                      CYPHER_FILESYSTEM_WATCH_DIRECTORY |
                                      CYPHER_FILESYSTEM_WATCH_RECURSIVE;
    if ( ( flags & ~allowed_flags ) != 0 ) {
        return fs_error_t::ERR_INVALID_FLAGS;
    }
    const bool wants_file = ( flags & CYPHER_FILESYSTEM_WATCH_FILE ) != 0u;
    const bool wants_directory = ( flags & CYPHER_FILESYSTEM_WATCH_DIRECTORY ) != 0u;
    const bool wants_recursive = ( flags & CYPHER_FILESYSTEM_WATCH_RECURSIVE ) != 0u;
    if ( !wants_file && !wants_directory ) {
        return fs_error_t::ERR_INVALID_FLAGS;
    }
    if ( wants_file && wants_directory ) {
        return fs_error_t::ERR_INVALID_ARGUMENT;
    }
    if ( wants_file && is_directory ) {
        return fs_error_t::ERR_NOT_FILE;
    }
    if ( wants_directory && !is_directory ) {
        return fs_error_t::ERR_NOT_DIRECTORY;
    }
    if ( wants_recursive && !is_directory ) {
        return fs_error_t::ERR_INVALID_ARGUMENT;
    }
    return fs_error_t::OK;
}       // ValidateWatchFlags

static watch_handle_t AllocateWatchHandle( runtime_state_t &state )
{
    watch_handle_t handle = state.next_watch_handle++;
    if ( handle == CYPHER_FILESYSTEM_INVALID_WATCH ) {
        handle = state.next_watch_handle++;
    }
    return handle;
}       // AllocateWatchHandle

static void ResetWatch( watch_t &watch )
{
    std::memset( &watch, 0, sizeof( watch ) );
#if defined( CYPHER_PLATFORM_LINUX )
    watch.native_fd = -1;
    watch.native_watch = -1;
#elif defined( CYPHER_PLATFORM_MACOS )
    watch.native_fd = -1;
#endif
}       // ResetWatch

static bool CopyString( char *out, common::u32 out_size, const char *text )
{
    if ( out == nullptr || out_size == 0u || text == nullptr ) {
        return false;
    }
    const common::usize len = std::strlen( text );
    if ( len + 1u > out_size ) {
        out[0] = '\0';
        return false;
    }
    std::memcpy( out, text, len + 1u );
    return true;
}       // CopyString

fs_error_t BuildWatchSnapshot( watch_t &watch )
{
    watch.snapshot_count = 0u;
    const bool watch_directory  = ( ( watch.flags & CYPHER_FILESYSTEM_WATCH_DIRECTORY ) != 0u );
    const bool watch_file       = ( ( watch.flags & CYPHER_FILESYSTEM_WATCH_FILE ) != 0u );
    const bool watch_recursive  = ( ( watch.flags & CYPHER_FILESYSTEM_WATCH_RECURSIVE ) != 0u );
    // @NOTE: Writing lambda function instead of using too many helpers.
    auto add_entry = [&]( const char *virtual_path, const std::filesystem::path &physical_path ) -> fs_error_t {
        if ( watch.snapshot_count >= CYPHER_FILESYSTEM_MAX_WATCH_SNAPSHOT_ENTRIES ) {
            return fs_error_t::ERR_BUFFER_TOO_SMALL;
        }
        watch_snapshot_entry_t &entry = watch.snapshot[watch.snapshot_count];
        entry = {};
        if ( !CopyString( entry.virtual_path, sizeof( entry.virtual_path ), virtual_path ) ||
             !CopyString( entry.physical_path, sizeof( entry.physical_path ), physical_path.string().c_str() ) )
        {
            return fs_error_t::ERR_BUFFER_TOO_SMALL;
        }
        std::error_code ec{};
        entry.exists = std::filesystem::exists( entry.physical_path, ec );
        if ( ec ) {
            return fs_error_t::ERR_IO_ERROR;
        }
        if ( entry.exists ) {
            entry.is_directory = std::filesystem::is_directory( physical_path, ec );
            if ( ec ) {
                return fs_error_t::ERR_IO_ERROR;
            }
            if ( !entry.is_directory ) {
                entry.size = static_cast<common::u64>( std::filesystem::file_size( physical_path, ec ) );
                if ( ec ) {
                    return fs_error_t::ERR_IO_ERROR;
                }

            }
            const auto file_time = std::filesystem::last_write_time( physical_path, ec );
            if ( !ec ) {
                const auto system_time = std::chrono::time_point_cast<std::chrono::system_clock::duration>( file_time - std::filesystem::file_time_type::clock::now() + std::chrono::system_clock::now() );
                entry.modified_time = std::chrono::system_clock::to_time_t( system_time );
            }
        }
        ++watch.snapshot_count;
        return fs_error_t::OK;
    };      // Lambda Function
    fs_error_t result = add_entry( watch.virtual_path, watch.physical_path );
    std::error_code ec{};
    if ( result != fs_error_t::OK ) {
        return result;
    }
    if ( watch_file ) {
        return fs_error_t::OK;
    }
    if ( watch_directory && !watch_recursive ) {
        // @NOTE-> otherwise continue scanning directory contents further more
        // This is non recursive first, recursive is later!
        for ( const std::filesystem::directory_entry &directory_entry : std::filesystem::directory_iterator( watch.physical_path, ec ) )
        {
            if ( ec ) {
                return fs_error_t::ERR_IO_ERROR;
            }
            // directory_entry.path() is the physical child path of the provided parent path
            std::string child_name = directory_entry.path().filename().generic_string();
            char child_virtual_path[CYPHER_FILESYSTEM_MAX_PATH_LENGTH]{};
            result = CypherFileSystem_PathJoin( watch.virtual_path, child_name.c_str(), child_virtual_path, sizeof( child_virtual_path ) );
            if ( result != fs_error_t::OK ) {
                return result;
            }
            result = add_entry( child_virtual_path, directory_entry.path() );
            if ( result != fs_error_t::OK ) {
                return result;
            }
        }
    }
    // @NOTE: Using recursive mode for watching things here.
    else if ( watch_directory && watch_recursive ) {
        // recursive watching now!
        for ( const std::filesystem::directory_entry &directory_entry : std::filesystem::recursive_directory_iterator( watch.physical_path, ec ) ) {
            if ( ec ) {
                return fs_error_t::ERR_IO_ERROR;
            }
            // @note: Now we need to compute the so called relative_path because there might be more folders with recursive ~ Karlo 15.6.2026
            std::filesystem::path relative_path = std::filesystem::relative( directory_entry.path(), watch.physical_path, ec );
            if ( ec ) {
                return fs_error_t::ERR_IO_ERROR;
            }
            std::string relative_string = relative_path.generic_string();
            char child_virtual_path[CYPHER_FILESYSTEM_MAX_PATH_LENGTH]{};
            result = CypherFileSystem_PathJoin( watch.virtual_path, relative_string.c_str(), child_virtual_path, sizeof( child_virtual_path ) );
            if ( result != fs_error_t::OK ) {
                return result;
            }
            result = add_entry( child_virtual_path, directory_entry.path() );
            if ( result != fs_error_t::OK ) {
                return result;
            }
        }
    }
    return fs_error_t::OK;
}

// @WindowsAPI Implementations ~ Karlo 17.06.2026
#if defined( CYPHER_PLATFORM_WINDOWS )

static void WindowsResetNativeWatch( windows_watch_t &win_watch )
{
    win_watch                   = {};
    win_watch.directory_handle  = INVALID_HANDLE_VALUE;
    win_watch.event_handle      = nullptr;
}

static windows_watch_t *WindowsAllocateNativeWatch()
{
    for ( common::u32 i = 0; i < CYPHER_FILESYSTEM_MAX_WATCHES; ++i )
    {
        windows_watch_t &win_watch = g_windows_watches[i];
        if ( !win_watch.used ) {
            WindowsResetNativeWatch( win_watch );
            win_watch.used = true;
            return &win_watch;
        }
    }
    return nullptr;
}

static windows_watch_t *WindowsGetNativeWatch( watch_t &watch )
{
    return static_cast<windows_watch_t *>( watch.native_handle );
}

static fs_error_t WindowsUtf8ToWide( const char *text, wchar_t *out, common::u32 out_size )
{
    if ( text == nullptr || out == nullptr || out_size == 0u ) {
        return fs_error_t::ERR_INVALID_ARGUMENT;
    }
    const int result = MultiByteToWideChar( CP_UTF8, MB_ERR_INVALID_CHARS, text, -1, out, static_cast<int>( out_size ) );
    if ( result == 0 ) {
        return fs_error_t::ERR_INVALID_PATH;
    }

    return fs_error_t::OK;
}

static void WindowsDestroyNativeWatch( watch_t &watch )
{
    windows_watch_t *win_watch = WindowsGetNativeWatch( watch );
    if ( win_watch == nullptr ) {
        return ;
    }
    if ( win_watch->read_pending && win_watch->directory_handle != INVALID_HANDLE_VALUE ) {
        CancelIoEx( win_watch->directory_handle, &win_watch->overlapped );
        win_watch->read_pending = false;
    }
    if ( win_watch->event_handle != nullptr ) {
        CloseHandle( win_watch->event_handle );
    }
    if ( win_watch->directory_handle != INVALID_HANDLE_VALUE ) {
        CloseHandle( win_watch->directory_handle );
    }
    WindowsResetNativeWatch( *win_watch );
    watch.native_handle = nullptr;
    return ;
}

static fs_error_t WindowsArmNativeWatch( watch_t &watch )
{
    windows_watch_t *win_watch = WindowsGetNativeWatch( watch );
    if ( win_watch == nullptr || !win_watch->used ) {
        return fs_error_t::ERR_INVALID_HANDLE;
    }
    if ( win_watch->read_pending ) {
        return fs_error_t::OK;
    }

    std::memset( &win_watch->overlapped, 0, sizeof( win_watch->overlapped ) );
    std::memset( win_watch->buffer, 0, sizeof( win_watch->buffer ) );

    ResetEvent( win_watch->event_handle );
    win_watch->overlapped.hEvent = win_watch->event_handle;

    const BOOL recursive = ( watch.flags & CYPHER_FILESYSTEM_WATCH_RECURSIVE ) != 0u ? TRUE : FALSE;
    const DWORD filter = FILE_NOTIFY_CHANGE_FILE_NAME   |
                         FILE_NOTIFY_CHANGE_DIR_NAME    |
                         FILE_NOTIFY_CHANGE_ATTRIBUTES  |
                         FILE_NOTIFY_CHANGE_SIZE        |
                         FILE_NOTIFY_CHANGE_LAST_WRITE  |
                         FILE_NOTIFY_CHANGE_CREATION;
    const BOOL ok = ReadDirectoryChangesW(
                    win_watch->directory_handle,
                    win_watch->buffer,
                    sizeof( win_watch->buffer ),
                    recursive,
                    filter,
                    nullptr,
                    &win_watch->overlapped,
                    nullptr
                    );
    if ( !ok ) {
        const DWORD error = GetLastError();
        if ( error != ERROR_IO_PENDING ) {
            return fs_error_t::ERR_IO_ERROR;
        }
    }
    win_watch->read_pending = true;
    return fs_error_t::OK;
}

static fs_error_t WindowsCreateNativeWatch( watch_t &watch )
{
    windows_watch_t *win_watch = WindowsAllocateNativeWatch();
    if ( win_watch == nullptr ) {
        return fs_error_t::ERR_TOO_MANY_WATCHES;
    }
    watch.native_handle = win_watch;

    const bool watch_is_file = ( watch.flags & CYPHER_FILESYSTEM_WATCH_FILE ) != 0u;
    const bool watch_is_directory = ( watch.flags & CYPHER_FILESYSTEM_WATCH_DIRECTORY ) != 0u;

    if ( watch_is_file ) {
        std::filesystem::path physical_path( watch.physical_path );
        std::string physical_parent = physical_path.parent_path().string();
        if ( !CopyString( win_watch->watch_physical_path, sizeof( win_watch->watch_physical_path ), physical_parent.c_str() ) ) {
            WindowsDestroyNativeWatch( watch );
            return fs_error_t::ERR_BUFFER_TOO_SMALL;
        }
        fs_error_t dirname_result = CypherFileSystem_PathDirname( watch.virtual_path, win_watch->watch_virtual_path, sizeof( win_watch->watch_virtual_path ) );
        if ( dirname_result != fs_error_t::OK ) {
            WindowsDestroyNativeWatch( watch );
            return dirname_result;
        }
        const char *basename = CypherFileSystem_PathBasename( watch.virtual_path );
        if ( basename == nullptr || basename[0] == '\0' ) {
            WindowsDestroyNativeWatch( watch );
            return fs_error_t::ERR_INVALID_PATH;
        }
        if ( !CopyString( win_watch->file_filter, sizeof( win_watch->file_filter ), basename ) ) {
            WindowsDestroyNativeWatch( watch );
            return fs_error_t::ERR_BUFFER_TOO_SMALL;
        }
        win_watch->watch_file = true;
    } else if ( watch_is_directory ) {
        if ( !CopyString( win_watch->watch_physical_path, sizeof( win_watch->watch_physical_path ), watch.physical_path ) ||
             !CopyString( win_watch->watch_virtual_path, sizeof( win_watch->watch_virtual_path ), watch.virtual_path ) ) {
            WindowsDestroyNativeWatch( watch );
            return fs_error_t::ERR_BUFFER_TOO_SMALL;
        }
        win_watch->file_filter[0] = '\0';
        win_watch->watch_file = false;
    } else {
        WindowsDestroyNativeWatch( watch );
        return fs_error_t::ERR_INVALID_FLAGS;
    }

    wchar_t wide_path[CYPHER_FILESYSTEM_MAX_PATH_LENGTH]{};
    fs_error_t wide_result = WindowsUtf8ToWide(
        win_watch->watch_physical_path,
        wide_path,
        static_cast<common::u32>( sizeof( wide_path ) / sizeof( wide_path[0] ) ) );

    if ( wide_result != fs_error_t::OK ) {
        WindowsDestroyNativeWatch( watch );
        return wide_result;
    }

    win_watch->directory_handle = CreateFileW(
        wide_path,
        FILE_LIST_DIRECTORY,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        nullptr,
        OPEN_EXISTING,
        FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
        nullptr );

    if ( win_watch->directory_handle == INVALID_HANDLE_VALUE ) {
        WindowsDestroyNativeWatch( watch );
        return fs_error_t::ERR_IO_ERROR;
    }

    win_watch->event_handle = CreateEventW( nullptr, TRUE, FALSE, nullptr );
    if ( win_watch->event_handle == nullptr ) {
        WindowsDestroyNativeWatch( watch );
        return fs_error_t::ERR_IO_ERROR;
    }

    fs_error_t arm_result = WindowsArmNativeWatch( watch );
    if ( arm_result != fs_error_t::OK ) {
        WindowsDestroyNativeWatch( watch );
        return arm_result;
    }

    return fs_error_t::OK;
}

#endif

}           // namespace

fs_error_t CypherFileSystem_WatchPath(
    const char *virtual_path,
    common::u32 flags,
    watch_handle_t &out_watch )
{
    std::lock_guard<std::recursive_mutex> lock( CypherFileSystem_RuntimeMutex() );
    runtime_state_t &state = CypherFileSystem_RuntimeState();
    out_watch = CYPHER_FILESYSTEM_INVALID_WATCH;
    if ( !state.initialized ) {
        return fs_error_t::ERR_NOT_INIT;
    }
    if ( virtual_path == nullptr || virtual_path[0] == '\0' ) {
        return fs_error_t::ERR_INVALID_PATH;
    }
    if ( state.watch_count >= CYPHER_FILESYSTEM_MAX_WATCHES ) {
        return fs_error_t::ERR_TOO_MANY_WATCHES;
    }
    char normalized_path[CYPHER_FILESYSTEM_MAX_PATH_LENGTH]{};
    char physical_path[CYPHER_FILESYSTEM_MAX_PATH_LENGTH]{};
    bool is_dir = false;
    fs_error_t result = ResolveWatchPhysicalPath( state, virtual_path, normalized_path, sizeof( normalized_path ), physical_path, sizeof( physical_path ), is_dir );
    if ( result != fs_error_t::OK ) {
        return result;
    }
    if ( flags == CYPHER_FILESYSTEM_WATCH_NONE ) {
        flags = is_dir ? CYPHER_FILESYSTEM_WATCH_DIRECTORY : CYPHER_FILESYSTEM_WATCH_FILE;
    }
    result = ValidateWatchFlags( flags, is_dir );
    if ( result != fs_error_t::OK ) {
        return result;
    }
    for ( common::u32 i = 0; i < state.watch_count; ++i ) {
        if ( std::strcmp( state.watches[i].virtual_path, normalized_path ) == 0 ) {
            return fs_error_t::ERR_ALREADY_EXISTS;
        }
    }
    watch_t &watch = state.watches[state.watch_count];
    ResetWatch( watch );
    watch.handle = AllocateWatchHandle( state );
    watch.flags = flags;

    if ( !CopyString( watch.virtual_path, sizeof( watch.virtual_path ), normalized_path ) ||
         !CopyString( watch.physical_path, sizeof( watch.physical_path ), physical_path ) )
    {
        return fs_error_t::ERR_BUFFER_TOO_SMALL;
    }
    result = BuildWatchSnapshot( watch );
    if ( result != fs_error_t::OK ) {
        ResetWatch( watch );
        return result;
    }

#if defined( CYPHER_PLATFORM_WINDOWS )
    result = WindowsCreateNativeWatch( watch );
    if ( result != fs_error_t::OK ) {
        ResetWatch( watch );
        return result;
    }
#endif

    ++state.watch_count;
    out_watch = watch.handle;

    return fs_error_t::OK;
}

fs_error_t CypherFileSystem_UnwatchPath( watch_handle_t watch_handle )
{
    std::lock_guard<std::recursive_mutex> lock( CypherFileSystem_RuntimeMutex() );
    runtime_state_t &state = CypherFileSystem_RuntimeState();
    if ( !state.initialized ) {
        return fs_error_t::ERR_NOT_INIT;
    }
    if ( watch_handle == CYPHER_FILESYSTEM_INVALID_WATCH ) {
        return fs_error_t::ERR_INVALID_HANDLE;
    }

    for ( common::u32 i = 0; i < state.watch_count; ++i ) {
        watch_t &watch = state.watches[i];
        if ( watch.handle == watch_handle ) {
#if defined( CYPHER_PLATFORM_WINDOWS )
            WindowsDestroyNativeWatch( watch );
#endif
            // @note -> shift all the watches and fill the holes
            for ( common::u32 j = i; j + 1 < state.watch_count; ++j ) {
                state.watches[j] = state.watches[j + 1];
            }
            --state.watch_count;
            ResetWatch( state.watches[state.watch_count] );
            return fs_error_t::OK;
        }
    }
    return fs_error_t::ERR_INVALID_HANDLE;
}

fs_error_t CypherFileSystem_PollChanges(
    watch_event_t *events,
    common::u32 max_events,
    common::u32 &out_event_count )
{
    std::lock_guard<std::recursive_mutex> lock( CypherFileSystem_RuntimeMutex() );
    runtime_state_t &state = CypherFileSystem_RuntimeState();
    out_event_count = 0u;
    if ( !state.initialized ) {
        return fs_error_t::ERR_NOT_INIT;
    }
    if ( events == nullptr && max_events != 0u ) {
        return fs_error_t::ERR_INVALID_ARGUMENT;
    }
    for ( common::u32 watch_idx = 0; watch_idx < state.watch_count; ++watch_idx ) {
        watch_t &old_watch = state.watches[watch_idx];
        watch_t &new_watch = state.watch_scratch;
        ResetWatch( new_watch );
        new_watch.handle = old_watch.handle;
        new_watch.flags = old_watch.flags;
        if ( !CopyString( new_watch.virtual_path, sizeof( new_watch.virtual_path ), old_watch.virtual_path ) ||
             !CopyString( new_watch.physical_path, sizeof( new_watch.physical_path ), old_watch.physical_path ) )
        {
            return fs_error_t::ERR_BUFFER_TOO_SMALL;
        }
        fs_error_t result = BuildWatchSnapshot( new_watch );
        if ( result != fs_error_t::OK ) {
            return result;
        }
        // @note: pass 1 -> we are checking the old passes and new passes and checking the differences
        for ( common::u32 old_watch_idx = 0; old_watch_idx < old_watch.snapshot_count; ++old_watch_idx ) {
            const watch_snapshot_entry_t &old_entry = old_watch.snapshot[old_watch_idx];

            bool found_in_new = false;

            for ( common::u32 new_watch_idx = 0; new_watch_idx < new_watch.snapshot_count; ++new_watch_idx ) {
                const watch_snapshot_entry_t &new_entry = new_watch.snapshot[new_watch_idx];
                if ( std::strcmp( old_entry.virtual_path, new_entry.virtual_path ) == 0 ) {
                    found_in_new = true;
                    // same virtual path exists in both snapshots
                    // now we compare the metadata.
                    if ( old_entry.size != new_entry.size ||
                         old_entry.modified_time != new_entry.modified_time ||
                         old_entry.is_directory != new_entry.is_directory ) {
                        // @NOTE: Here we emit MODIFIED for the old_entry.virtual_path
                        if ( out_event_count >= max_events ) {
                            return fs_error_t::ERR_BUFFER_TOO_SMALL;
                        }
                        watch_event_t &event = events[out_event_count];
                        event = {};
                        event.type = watch_event_type_t::MODIFIED;
                        if ( !CopyString( event.virtual_path, sizeof( event.virtual_path ), old_entry.virtual_path ) ) {
                            return fs_error_t::ERR_BUFFER_TOO_SMALL;
                        }
                        event.old_virtual_path[0] = '\0';
                        ++out_event_count;
                    }
                    break;
                }
            }
            if ( !found_in_new ) {
                // @note: emit DELETED for old_entry.virtual_path!
                if ( out_event_count >= max_events ) {
                    return fs_error_t::ERR_BUFFER_TOO_SMALL;
                }
                watch_event_t &event = events[out_event_count];
                event = {};
                event.type = watch_event_type_t::DELETED;
                if ( !CopyString( event.virtual_path, sizeof( event.virtual_path ), old_entry.virtual_path ) ) {
                    return fs_error_t::ERR_BUFFER_TOO_SMALL;
                }
                event.old_virtual_path[0] = '\0';
                ++out_event_count;
            }
        }
        // @note: Pass 2 -> we are chekcing the new passes and the old passes and checking the differences, we need to know if this new entry existed beforehand.
        for ( common::u32 new_watch_idx = 0; new_watch_idx < new_watch.snapshot_count; ++new_watch_idx ) {
            const watch_snapshot_entry_t &new_entry = new_watch.snapshot[new_watch_idx];
            bool found_in_old = false;
            for ( common::u32 old_watch_idx = 0; old_watch_idx < old_watch.snapshot_count; ++old_watch_idx ) {
                const watch_snapshot_entry_t &old_entry = old_watch.snapshot[old_watch_idx];
                if ( std::strcmp( new_entry.virtual_path, old_entry.virtual_path ) == 0 ) {
                    found_in_old = true;
                    break;
                }
            }
            if ( !found_in_old ) {
                // @note -> we here emit CREATED for the new_entry.virtual_path
                if ( out_event_count >= max_events ) {
                    return fs_error_t::ERR_BUFFER_TOO_SMALL;
                }
                watch_event_t &event = events[out_event_count];
                event = {};
                event.type = watch_event_type_t::CREATED;
                if ( !CopyString( event.virtual_path,
                                  sizeof( event.virtual_path ),
                                  new_entry.virtual_path ) ) {
                    return fs_error_t::ERR_BUFFER_TOO_SMALL;
                }
                event.old_virtual_path[0] = '\0';
                ++out_event_count;
            }
        }
        old_watch.snapshot_count = new_watch.snapshot_count;
        for ( common::u32 snapshot_idx = 0; snapshot_idx < new_watch.snapshot_count; ++snapshot_idx ) {
            old_watch.snapshot[snapshot_idx] = new_watch.snapshot[snapshot_idx];
        }
    }
    return fs_error_t::OK;
}

}       // namespace cypher::engine::fs

#include "CypherEngine/CypherFileSystem/CypherFileSystem.h"

#include <cstdio>
#include <cstring>
#include <filesystem>

namespace fs = cypher::engine::fs;

namespace {

bool Check( bool condition, const char *message )
{
    if ( condition ) {
        return true;
    }

    std::fprintf( stderr, "filesystem smoke failed: %s\n", message );
    return false;
}

bool CheckError( fs::fs_error_t actual, fs::fs_error_t expected, const char *message )
{
    if ( actual == expected ) {
        return true;
    }

    std::fprintf(
        stderr,
        "filesystem smoke failed: %s: expected %s, got %s\n",
        message,
        fs::CypherFileSystem_ErrorName( expected ),
        fs::CypherFileSystem_ErrorName( actual ) );
    return false;
}

}       // namespace

int main()
{
    const std::filesystem::path root_path =
        std::filesystem::temp_directory_path() / "CypherFileSystem_Smoke";

    std::error_code ec{};
    std::filesystem::remove_all( root_path, ec );
    std::filesystem::create_directories( root_path, ec );

    if ( ec ) {
        std::fprintf( stderr, "filesystem smoke failed: temp directory setup failed\n" );
        return 1;
    }

    const std::string write_path = root_path.string();

    if ( !CheckError( fs::CypherFileSystem_Init(), fs::fs_error_t::OK, "init" ) ) {
        return 1;
    }
    if ( !CheckError( fs::CypherFileSystem_SetWritePath( write_path.c_str() ), fs::fs_error_t::OK, "set write path" ) ) {
        return 1;
    }
    if ( !CheckError( fs::CypherFileSystem_CreateDirectory( "profiles/player1" ), fs::fs_error_t::OK, "create directory" ) ) {
        return 1;
    }

    fs::file_t file{};
    if ( !CheckError( fs::CypherFileSystem_Open( "profiles/player1/config.cfg", fs::open_mode_t::WRITE_TEXT, file ), fs::fs_error_t::OK, "open write" ) ) {
        return 1;
    }

    const char text[] = "name player1\n";
    cypher::engine::common::u64 bytes_written = 0u;
    if ( !CheckError( fs::CypherFileSystem_Write( file, text, std::strlen( text ), bytes_written ), fs::fs_error_t::OK, "write file" ) ) {
        return 1;
    }
    if ( !Check( bytes_written == std::strlen( text ), "write count" ) ) {
        return 1;
    }
    if ( !CheckError( fs::CypherFileSystem_Close( file ), fs::fs_error_t::OK, "close file" ) ) {
        return 1;
    }

    if ( !CheckError( fs::CypherFileSystem_MountDirectory( "", write_path.c_str(), fs::CYPHER_FILESYSTEM_MOUNT_READ_ONLY, 0u ), fs::fs_error_t::OK, "mount write path for read view" ) ) {
        return 1;
    }
    if ( !Check( fs::CypherFileSystem_Exists( "profiles/player1/config.cfg" ), "file exists through mount" ) ) {
        return 1;
    }
    if ( !Check( fs::CypherFileSystem_DirectoryExists( "profiles/player1" ), "directory exists through mount" ) ) {
        return 1;
    }

    if ( !CheckError( fs::CypherFileSystem_Rename( "profiles/player1/config.cfg", "profiles/player1/config2.cfg" ), fs::fs_error_t::OK, "rename file" ) ) {
        return 1;
    }
    if ( !Check( fs::CypherFileSystem_Exists( "profiles/player1/config2.cfg" ), "renamed file exists through mount" ) ) {
        return 1;
    }
    if ( !CheckError( fs::CypherFileSystem_DeleteFile( "profiles/player1/config2.cfg" ), fs::fs_error_t::OK, "delete file" ) ) {
        return 1;
    }
    if ( !CheckError( fs::CypherFileSystem_RemoveDirectory( "profiles/player1" ), fs::fs_error_t::OK, "remove player directory" ) ) {
        return 1;
    }
    if ( !CheckError( fs::CypherFileSystem_RemoveDirectory( "profiles" ), fs::fs_error_t::OK, "remove profiles directory" ) ) {
        return 1;
    }
    if ( !CheckError( fs::CypherFileSystem_Shutdown(), fs::fs_error_t::OK, "shutdown" ) ) {
        return 1;
    }

    std::filesystem::remove_all( root_path, ec );
    return 0;
}

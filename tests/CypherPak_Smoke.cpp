#include "CypherEngine/CypherPak/CypherPak.h"

#include <cstdio>
#include <cstring>
#include <filesystem>
#include <string>

namespace pak = cypher::engine::pak;

namespace {

bool Check( const bool condition, const char *message )
{
    if ( condition ) {
        return true;
    }

    std::fprintf( stderr, "pak smoke failed: %s\n", message );
    return false;
}

bool CheckError( const pak::pak_error_t actual, const pak::pak_error_t expected, const char *message )
{
    if ( actual == expected ) {
        return true;
    }

    std::fprintf(
        stderr,
        "pak smoke failed: %s: expected %s, got %s\n",
        message,
        pak::CypherPak_ErrorName( expected ),
        pak::CypherPak_ErrorName( actual ) );
    return false;
}

bool WritePhysicalFile( const std::filesystem::path &path, const void *data, const std::size_t size )
{
    std::error_code ec{};
    std::filesystem::create_directories( path.parent_path(), ec );
    if ( ec ) {
        return false;
    }

    std::FILE *file = std::fopen( path.string().c_str(), "wb" );
    if ( file == nullptr ) {
        return false;
    }

    const bool write_ok = size == 0u || std::fwrite( data, 1u, size, file ) == size;
    const bool close_ok = std::fclose( file ) == 0;
    return write_ok && close_ok;
}

bool WritePhysicalTextFile( const std::filesystem::path &path, const char *text )
{
    return WritePhysicalFile( path, text, std::strlen( text ) );
}

bool WriteMinimalArchiveHeader(
    const std::filesystem::path &path,
    const bool valid_magic,
    const cypher::engine::common::u32 version )
{
    unsigned char header_bytes[pak::CYPHER_PAK_HEADER_SIZE]{};
    std::size_t offset = 0u;

    if ( valid_magic ) {
        std::memcpy( header_bytes + offset, pak::CYPHER_PAK_MAGIC, pak::CYPHER_PAK_MAGIC_SIZE );
    } else {
        std::memcpy( header_bytes + offset, "NOTCYPACKAGE", 12u );
    }
    offset += pak::CYPHER_PAK_MAGIC_SIZE;

    pak::CypherPak_StoreU32LE( header_bytes + offset, version );
    offset += sizeof( cypher::engine::common::u32 );
    pak::CypherPak_StoreU32LE( header_bytes + offset, pak::CYPHER_PAK_HEADER_SIZE );
    offset += sizeof( cypher::engine::common::u32 );
    pak::CypherPak_StoreU32LE( header_bytes + offset, pak::CYPHER_PAK_ENDIAN_TAG );
    offset += sizeof( cypher::engine::common::u32 );
    pak::CypherPak_StoreU32LE( header_bytes + offset, pak::CYPHER_PAK_FORMAT_NONE );
    offset += sizeof( cypher::engine::common::u32 );

    pak::CypherPak_StoreU64LE( header_bytes + offset, pak::CYPHER_PAK_HEADER_SIZE );
    offset += sizeof( cypher::engine::common::u64 );
    pak::CypherPak_StoreU64LE( header_bytes + offset, 0u );
    offset += sizeof( cypher::engine::common::u64 );

    pak::CypherPak_StoreU64LE( header_bytes + offset, pak::CYPHER_PAK_HEADER_SIZE );
    offset += sizeof( cypher::engine::common::u64 );
    pak::CypherPak_StoreU64LE( header_bytes + offset, 0u );
    offset += sizeof( cypher::engine::common::u64 );
    pak::CypherPak_StoreU64LE( header_bytes + offset, pak::CYPHER_PAK_HEADER_SIZE );
    offset += sizeof( cypher::engine::common::u64 );
    pak::CypherPak_StoreU64LE( header_bytes + offset, 0u );
    offset += sizeof( cypher::engine::common::u64 );
    pak::CypherPak_StoreU64LE( header_bytes + offset, pak::CYPHER_PAK_HEADER_SIZE );
    offset += sizeof( cypher::engine::common::u64 );
    pak::CypherPak_StoreU64LE( header_bytes + offset, 0u );

    return WritePhysicalFile( path, header_bytes, sizeof( header_bytes ) );
}

}       // namespace

int main()
{
    const std::filesystem::path root_path =
        std::filesystem::temp_directory_path() / "CypherPak_Smoke";

    std::error_code ec{};
    std::filesystem::remove_all( root_path, ec );
    std::filesystem::create_directories( root_path, ec );
    if ( ec ) {
        std::fprintf( stderr, "pak smoke failed: temp directory setup failed\n" );
        return 1;
    }

    const std::filesystem::path source_path = root_path / "Source";
    const std::filesystem::path player_cfg_path = source_path / "Player.cfg";
    const std::filesystem::path wall_path = source_path / "Wall.dds";
    const std::filesystem::path empty_path = source_path / "Empty.bin";
    const std::filesystem::path archive_path = root_path / "game.cypak";
    const std::filesystem::path archive_path_2 = root_path / "game_incremental.cypak";
    const std::filesystem::path duplicate_archive_path = root_path / "duplicate.cypak";
    const std::filesystem::path bad_magic_archive_path = root_path / "bad_magic.cypak";
    const std::filesystem::path wrong_version_archive_path = root_path / "wrong_version.cypak";
    const std::filesystem::path truncated_archive_path = root_path / "truncated.cypak";

    const std::string player_cfg_path_string = player_cfg_path.string();
    const std::string wall_path_string = wall_path.string();
    const std::string empty_path_string = empty_path.string();
    const std::string archive_path_string = archive_path.string();
    const std::string archive_path_2_string = archive_path_2.string();
    const std::string duplicate_archive_path_string = duplicate_archive_path.string();
    const std::string bad_magic_archive_path_string = bad_magic_archive_path.string();
    const std::string wrong_version_archive_path_string = wrong_version_archive_path.string();
    const std::string truncated_archive_path_string = truncated_archive_path.string();

    const char player_cfg[] = "name player1\nrate 1\n";
    const unsigned char wall_data[] = { 0x13u, 0x37u, 0xC0u, 0xDEu, 0x00u, 0x42u };

    if ( !Check( WritePhysicalTextFile( player_cfg_path, player_cfg ), "write player config source" ) ) {
        return 1;
    }
    if ( !Check( WritePhysicalFile( wall_path, wall_data, sizeof( wall_data ) ), "write wall source" ) ) {
        return 1;
    }
    if ( !Check( WritePhysicalFile( empty_path, "", 0u ), "write empty source" ) ) {
        return 1;
    }

    if ( !Check( WriteMinimalArchiveHeader( bad_magic_archive_path, false, pak::CYPHER_PAK_FORMAT_VERSION ), "write bad magic archive" ) ) {
        return 1;
    }
    if ( !Check( WriteMinimalArchiveHeader( wrong_version_archive_path, true, pak::CYPHER_PAK_FORMAT_VERSION + 1u ), "write wrong version archive" ) ) {
        return 1;
    }
    const unsigned char truncated_bytes[] = { 'C', 'Y', 'P', 'A' };
    if ( !Check( WritePhysicalFile( truncated_archive_path, truncated_bytes, sizeof( truncated_bytes ) ), "write truncated archive" ) ) {
        return 1;
    }

    const pak::pak_source_file_t source_files[] = {
        { "Scripts\\Player.CFG", player_cfg_path_string.c_str(), pak::pak_compression_t::NONE, pak::CYPHER_PAK_ENTRY_NONE },
        { "textures/Wall.DDS", wall_path_string.c_str(), pak::pak_compression_t::NONE, pak::CYPHER_PAK_ENTRY_NONE },
        { "empty/file.bin", empty_path_string.c_str(), pak::pak_compression_t::NONE, pak::CYPHER_PAK_ENTRY_NONE }
    };

    pak::pak_writer_config_t config{};
    config.archive_path = archive_path_string.c_str();
    if ( !CheckError( pak::CypherPak_CreateArchive( config, source_files, 3u ), pak::pak_error_t::OK, "create archive" ) ) {
        return 1;
    }
    if ( !Check( std::filesystem::exists( archive_path, ec ), "archive exists" ) ) {
        return 1;
    }

    pak::pak_reader_t reader{};
    if ( !CheckError( pak::CypherPak_OpenReader( bad_magic_archive_path_string.c_str(), pak::CYPHER_PAK_OPEN_NONE, reader ), pak::pak_error_t::ERR_BAD_MAGIC, "open bad magic archive" ) ) {
        return 1;
    }
    if ( !CheckError( pak::CypherPak_OpenReader( wrong_version_archive_path_string.c_str(), pak::CYPHER_PAK_OPEN_NONE, reader ), pak::pak_error_t::ERR_UNSUPPORTED_VERSION, "open wrong version archive" ) ) {
        return 1;
    }
    if ( !CheckError( pak::CypherPak_OpenReader( truncated_archive_path_string.c_str(), pak::CYPHER_PAK_OPEN_NONE, reader ), pak::pak_error_t::ERR_FILE_READ_FAILED, "open truncated archive" ) ) {
        return 1;
    }

    cypher::engine::common::u32 unopened_file_count = 99u;
    if ( !CheckError( pak::CypherPak_GetFileCount( reader, unopened_file_count ), pak::pak_error_t::ERR_INVALID_HANDLE, "unopened reader file count" ) ) {
        return 1;
    }
    if ( !Check( unopened_file_count == 0u, "unopened reader file count reset" ) ) {
        return 1;
    }
    pak::pak_file_index_t unopened_index = 0u;
    if ( !CheckError( pak::CypherPak_FindFile( reader, "scripts/player.cfg", unopened_index ), pak::pak_error_t::ERR_INVALID_HANDLE, "unopened reader find" ) ) {
        return 1;
    }
    if ( !Check( unopened_index == pak::CYPHER_PAK_INVALID_FILE_INDEX, "unopened reader find index reset" ) ) {
        return 1;
    }
    char unopened_buffer[8]{};
    cypher::engine::common::u64 unopened_bytes_read = 99u;
    if ( !CheckError( pak::CypherPak_ReadFile( reader, "scripts/player.cfg", unopened_buffer, sizeof( unopened_buffer ), unopened_bytes_read ), pak::pak_error_t::ERR_INVALID_HANDLE, "unopened reader read" ) ) {
        return 1;
    }
    if ( !Check( unopened_bytes_read == 0u, "unopened reader read byte count reset" ) ) {
        return 1;
    }

    if ( !CheckError( pak::CypherPak_OpenReader( archive_path_string.c_str(), pak::CYPHER_PAK_OPEN_VERIFY_FILE_HASHES, reader ), pak::pak_error_t::OK, "open reader" ) ) {
        return 1;
    }
    if ( !Check( reader.header.version == pak::CYPHER_PAK_FORMAT_VERSION, "reader version" ) ) {
        return 1;
    }
    if ( !Check( pak::CypherPak_MagicEquals( reader.header.magic ), "reader magic" ) ) {
        return 1;
    }

    cypher::engine::common::u32 file_count = 0u;
    if ( !CheckError( pak::CypherPak_GetFileCount( reader, file_count ), pak::pak_error_t::OK, "get file count" ) ) {
        return 1;
    }
    if ( !Check( file_count == 3u, "file count value" ) ) {
        return 1;
    }

    pak::pak_file_index_t player_index = pak::CYPHER_PAK_INVALID_FILE_INDEX;
    if ( !CheckError( pak::CypherPak_FindFile( reader, "SCRIPTS\\PLAYER.CFG", player_index ), pak::pak_error_t::OK, "find normalized file" ) ) {
        return 1;
    }

    pak::pak_file_info_t player_info{};
    if ( !CheckError( pak::CypherPak_GetFileInfo( reader, player_index, player_info ), pak::pak_error_t::OK, "get file info by index" ) ) {
        return 1;
    }
    if ( !Check( std::strcmp( player_info.virtual_path, "scripts/player.cfg" ) == 0, "normalized file info path" ) ) {
        return 1;
    }
    if ( !Check( player_info.stored_size == std::strlen( player_cfg ), "stored size" ) ) {
        return 1;
    }

    char read_buffer[128]{};
    cypher::engine::common::u64 bytes_read = 0u;
    if ( !CheckError( pak::CypherPak_ReadFile( reader, "scripts/player.cfg", read_buffer, sizeof( read_buffer ), bytes_read ), pak::pak_error_t::OK, "read file" ) ) {
        return 1;
    }
    if ( !Check( bytes_read == std::strlen( player_cfg ) && std::memcmp( read_buffer, player_cfg, std::strlen( player_cfg ) ) == 0, "read file bytes" ) ) {
        return 1;
    }

    if ( !CheckError( pak::CypherPak_ReadFile( reader, "textures/wall.dds", read_buffer, 2u, bytes_read ), pak::pak_error_t::ERR_BUFFER_TOO_SMALL, "read buffer too small" ) ) {
        return 1;
    }
    if ( !CheckError( pak::CypherPak_FindFile( reader, "missing/file.txt", player_index ), pak::pak_error_t::ERR_ENTRY_NOT_FOUND, "missing file" ) ) {
        return 1;
    }

    pak::pak_stats_t stats{};
    if ( !CheckError( pak::CypherPak_GetStats( reader, stats ), pak::pak_error_t::OK, "get stats" ) ) {
        return 1;
    }
    if ( !Check( stats.file_count == 3u && stats.stored_data_size >= sizeof( wall_data ), "stats values" ) ) {
        return 1;
    }

    if ( !CheckError( pak::CypherPak_Verify( reader, pak::CYPHER_PAK_VERIFY_FULL ), pak::pak_error_t::OK, "verify full" ) ) {
        return 1;
    }
    if ( !CheckError( pak::CypherPak_CloseReader( reader ), pak::pak_error_t::OK, "close reader" ) ) {
        return 1;
    }

    pak::pak_writer_config_t incremental_config{};
    incremental_config.archive_path = archive_path_2_string.c_str();
    pak::pak_writer_t writer{};
    if ( !CheckError( pak::CypherPak_BeginWriter( incremental_config, writer ), pak::pak_error_t::OK, "begin writer" ) ) {
        return 1;
    }
    if ( !CheckError( pak::CypherPak_AddFile( writer, source_files[0] ), pak::pak_error_t::OK, "add file" ) ) {
        return 1;
    }
    if ( !CheckError( pak::CypherPak_FinishWriter( writer ), pak::pak_error_t::OK, "finish writer" ) ) {
        return 1;
    }

    if ( !CheckError( pak::CypherPak_OpenReader( archive_path_2_string.c_str(), pak::CYPHER_PAK_OPEN_VERIFY_FILE_HASHES, reader ), pak::pak_error_t::OK, "open incremental archive" ) ) {
        return 1;
    }
    if ( !CheckError( pak::CypherPak_GetFileCount( reader, file_count ), pak::pak_error_t::OK, "get incremental file count" ) ) {
        return 1;
    }
    if ( !Check( file_count == 1u, "incremental file count value" ) ) {
        return 1;
    }
    if ( !CheckError( pak::CypherPak_CloseReader( reader ), pak::pak_error_t::OK, "close incremental reader" ) ) {
        return 1;
    }

    const pak::pak_source_file_t duplicate_files[] = {
        { "duplicate/file.txt", player_cfg_path_string.c_str(), pak::pak_compression_t::NONE, pak::CYPHER_PAK_ENTRY_NONE },
        { "DUPLICATE\\FILE.TXT", player_cfg_path_string.c_str(), pak::pak_compression_t::NONE, pak::CYPHER_PAK_ENTRY_NONE }
    };
    config.archive_path = duplicate_archive_path_string.c_str();
    if ( !CheckError( pak::CypherPak_CreateArchive( config, duplicate_files, 2u ), pak::pak_error_t::ERR_DUPLICATE_ENTRY, "duplicate normalized path" ) ) {
        return 1;
    }

    pak::pak_compression_config_t compression_config{};
    unsigned char compressed[16]{};
    cypher::engine::common::u64 compressed_size = 0u;
    if ( !CheckError( pak::CypherPak_Compress( compression_config, wall_data, sizeof( wall_data ), compressed, sizeof( compressed ), compressed_size ), pak::pak_error_t::OK, "none compression copy" ) ) {
        return 1;
    }
    if ( !Check( compressed_size == sizeof( wall_data ) && std::memcmp( compressed, wall_data, sizeof( wall_data ) ) == 0, "none compression bytes" ) ) {
        return 1;
    }
    cypher::engine::common::u64 bound = 0u;
    if ( !CheckError( pak::CypherPak_CompressBound( pak::pak_compression_t::LZ4, sizeof( wall_data ), bound ), pak::pak_error_t::ERR_UNSUPPORTED_COMPRESSION, "lz4 unsupported for now" ) ) {
        return 1;
    }

    std::filesystem::remove_all( root_path, ec );
    return 0;
}

#include "CypherEngine/CypherPak/CypherPak_Reader.h"

#include <algorithm>
#include <atomic>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <limits>
#include <new>
#include <string>
#include <vector>

#if defined( _WIN32 )
#include <cstdio>
#else
#include <sys/types.h>
#endif

namespace cypher::engine::pak
{

namespace {

constexpr common::u32 FNV1A32_OFFSET = 2166136261u;
constexpr common::u32 FNV1A32_PRIME = 16777619u;
constexpr common::u64 FNV1A64_OFFSET = 14695981039346656037ull;
constexpr common::u64 FNV1A64_PRIME = 1099511628211ull;
constexpr common::u64 READ_CHUNK_SIZE = 64u * 1024u;

std::atomic<common::u32> g_next_reader_handle{ 1u };

pak_handle_t AllocateReaderHandle()
{
    common::u32 handle = g_next_reader_handle.fetch_add( 1u );
    if ( handle == CYPHER_PAK_INVALID_HANDLE ) {
        handle = g_next_reader_handle.fetch_add( 1u );
    }
    return handle;
}

bool CopyString( char *out, const common::u32 out_size, const char *text )
{
    if ( out == nullptr || out_size == 0u || text == nullptr ) {
        return false;
    }

    const common::usize text_len = std::strlen( text );
    if ( text_len + 1u > out_size ) {
        out[0] = '\0';
        return false;
    }

    std::memcpy( out, text, text_len + 1u );
    return true;
}

bool IsAsciiAlpha( const char c )
{
    return ( c >= 'a' && c <= 'z' ) || ( c >= 'A' && c <= 'Z' );
}

char ToLowerAscii( const char c )
{
    if ( c >= 'A' && c <= 'Z' ) {
        return static_cast<char>( c - 'A' + 'a' );
    }
    return c;
}

bool IsInvalidPathChar( const char c )
{
    const unsigned char u = static_cast<unsigned char>( c );
    if ( u < 32u || u == 127u || u >= 128u ) {
        return true;
    }

    switch ( c ) {
    case ':':
    case '"':
    case '<':
    case '>':
    case '|':
    case ' ':
    case '*':
    case '?':
        return true;
    default:
        return false;
    }
}

pak_error_t NormalizeVirtualPath( const char *virtual_path, char *out_path, const common::u32 out_path_size )
{
    if ( virtual_path == nullptr || virtual_path[0] == '\0' ) {
        return pak_error_t::ERR_INVALID_PATH;
    }
    if ( out_path == nullptr || out_path_size == 0u ) {
        return pak_error_t::ERR_INVALID_ARGUMENT;
    }

    out_path[0] = '\0';

    if ( virtual_path[0] == '/' || virtual_path[0] == '\\' ) {
        return pak_error_t::ERR_INVALID_PATH;
    }
    if ( IsAsciiAlpha( virtual_path[0] ) && virtual_path[1] == ':' ) {
        return pak_error_t::ERR_INVALID_PATH;
    }

    common::u32 write_index = 0u;
    common::u32 segment_count = 0u;
    const char *cursor = virtual_path;

    while ( *cursor != '\0' ) {
        while ( *cursor == '/' || *cursor == '\\' ) {
            ++cursor;
        }
        if ( *cursor == '\0' ) {
            break;
        }

        const char *segment_start = cursor;
        while ( *cursor != '\0' && *cursor != '/' && *cursor != '\\' ) {
            if ( IsInvalidPathChar( *cursor ) ) {
                out_path[0] = '\0';
                return pak_error_t::ERR_INVALID_PATH;
            }
            ++cursor;
        }

        const common::usize segment_len = static_cast<common::usize>( cursor - segment_start );
        if ( segment_len == 0u ) {
            continue;
        }
        if ( ( segment_len == 1u && segment_start[0] == '.' ) ||
             ( segment_len == 2u && segment_start[0] == '.' && segment_start[1] == '.' ) ) {
            out_path[0] = '\0';
            return pak_error_t::ERR_INVALID_PATH;
        }
        if ( segment_count > 0u ) {
            if ( write_index + 1u >= out_path_size ) {
                out_path[0] = '\0';
                return pak_error_t::ERR_BUFFER_TOO_SMALL;
            }
            out_path[write_index++] = '/';
        }
        for ( common::usize i = 0u; i < segment_len; ++i ) {
            if ( write_index + 1u >= out_path_size ) {
                out_path[0] = '\0';
                return pak_error_t::ERR_BUFFER_TOO_SMALL;
            }
            out_path[write_index++] = ToLowerAscii( segment_start[i] );
        }

        ++segment_count;
    }

    if ( segment_count == 0u ) {
        return pak_error_t::ERR_INVALID_PATH;
    }

    out_path[write_index] = '\0';
    return pak_error_t::OK;
}

common::u32 HashPath32( const char *path )
{
    common::u32 hash = FNV1A32_OFFSET;
    const unsigned char *cursor = reinterpret_cast<const unsigned char *>( path );
    while ( *cursor != 0u ) {
        hash ^= static_cast<common::u32>( *cursor++ );
        hash *= FNV1A32_PRIME;
    }
    return hash;
}

common::u64 HashBytes64( const void *data, const common::u64 size, common::u64 hash = FNV1A64_OFFSET )
{
    const common::u8 *bytes = static_cast<const common::u8 *>( data );
    for ( common::u64 i = 0u; i < size; ++i ) {
        hash ^= static_cast<common::u64>( bytes[i] );
        hash *= FNV1A64_PRIME;
    }
    return hash;
}

bool AddOverflow( const common::u64 a, const common::u64 b, common::u64 &out )
{
    out = a + b;
    return out < a;
}

bool RangeInside( const common::u64 offset, const common::u64 size, const common::u64 container_size )
{
    common::u64 end = 0u;
    if ( AddOverflow( offset, size, end ) ) {
        return false;
    }
    return end <= container_size;
}

bool OpenFlagsSupported( const common::u32 flags )
{
    const common::u32 supported_flags =
        CYPHER_PAK_OPEN_VERIFY_HEADER |
        CYPHER_PAK_OPEN_VERIFY_INDEX |
        CYPHER_PAK_OPEN_VERIFY_FILE_HASHES |
        CYPHER_PAK_OPEN_MEMORY_MAP;
    return ( flags & ~supported_flags ) == 0u;
}

bool SeekFile( std::FILE *file, const common::u64 offset )
{
    if ( file == nullptr ) {
        return false;
    }

#if defined( _WIN32 )
    return _fseeki64( file, static_cast<__int64>( offset ), SEEK_SET ) == 0;
#else
    if ( offset > static_cast<common::u64>( std::numeric_limits<off_t>::max() ) ) {
        return false;
    }
    return ::fseeko( file, static_cast<off_t>( offset ), SEEK_SET ) == 0;
#endif
}

bool ReadExact( std::FILE *file, void *buffer, const common::u64 size )
{
    if ( size == 0u ) {
        return true;
    }
    if ( file == nullptr || buffer == nullptr ) {
        return false;
    }

    common::u8 *write = static_cast<common::u8 *>( buffer );
    common::u64 remaining = size;
    while ( remaining > 0u ) {
        const common::u64 chunk64 = remaining < READ_CHUNK_SIZE ? remaining : READ_CHUNK_SIZE;
        const common::usize chunk = static_cast<common::usize>( chunk64 );
        if ( std::fread( write, 1u, chunk, file ) != chunk ) {
            return false;
        }
        write += chunk;
        remaining -= chunk64;
    }

    return true;
}

pak_header_t DecodeHeader( const common::u8 *bytes )
{
    pak_header_t header{};
    common::usize offset = 0u;

    std::memcpy( header.magic, bytes + offset, CYPHER_PAK_MAGIC_SIZE );
    offset += CYPHER_PAK_MAGIC_SIZE;

    header.version = CypherPak_LoadU32LE( bytes + offset );
    offset += 4u;
    header.header_size = CypherPak_LoadU32LE( bytes + offset );
    offset += 4u;
    header.endian_tag = CypherPak_LoadU32LE( bytes + offset );
    offset += 4u;
    header.flags = CypherPak_LoadU32LE( bytes + offset );
    offset += 4u;

    header.archive_size = CypherPak_LoadU64LE( bytes + offset );
    offset += 8u;
    header.file_count = CypherPak_LoadU64LE( bytes + offset );
    offset += 8u;
    header.index_offset = CypherPak_LoadU64LE( bytes + offset );
    offset += 8u;
    header.index_size = CypherPak_LoadU64LE( bytes + offset );
    offset += 8u;
    header.string_table_offset = CypherPak_LoadU64LE( bytes + offset );
    offset += 8u;
    header.string_table_size = CypherPak_LoadU64LE( bytes + offset );
    offset += 8u;
    header.data_offset = CypherPak_LoadU64LE( bytes + offset );
    offset += 8u;
    header.data_size = CypherPak_LoadU64LE( bytes + offset );
    offset += 8u;
    header.archive_hash = CypherPak_LoadU64LE( bytes + offset );
    offset += 8u;

    for ( common::u32 i = 0u; i < 4u; ++i ) {
        header.reserved[i] = CypherPak_LoadU64LE( bytes + offset );
        offset += 8u;
    }

    return header;
}

pak_disk_file_entry_t DecodeFileEntry( const common::u8 *bytes )
{
    pak_disk_file_entry_t entry{};
    common::usize offset = 0u;

    entry.path_offset = CypherPak_LoadU64LE( bytes + offset );
    offset += 8u;
    entry.data_offset = CypherPak_LoadU64LE( bytes + offset );
    offset += 8u;
    entry.stored_size = CypherPak_LoadU64LE( bytes + offset );
    offset += 8u;
    entry.unpacked_size = CypherPak_LoadU64LE( bytes + offset );
    offset += 8u;
    entry.modified_time_utc = CypherPak_LoadU64LE( bytes + offset );
    offset += 8u;
    entry.content_hash = CypherPak_LoadU64LE( bytes + offset );
    offset += 8u;
    entry.path_size = CypherPak_LoadU32LE( bytes + offset );
    offset += 4u;
    entry.path_hash = CypherPak_LoadU32LE( bytes + offset );
    offset += 4u;
    entry.compression = CypherPak_LoadU32LE( bytes + offset );
    offset += 4u;
    entry.flags = CypherPak_LoadU32LE( bytes + offset );

    return entry;
}

const char *EntryPath( const pak_reader_t &reader, const pak_disk_file_entry_t &entry )
{
    if ( reader.string_table == nullptr || entry.path_offset >= reader.string_table_size ) {
        return nullptr;
    }
    return reader.string_table + entry.path_offset;
}

pak_error_t ValidateLoadedIndex( const pak_reader_t &reader )
{
    if ( reader.header.file_count != reader.file_count ) {
        return pak_error_t::ERR_INVALID_INDEX;
    }
    if ( reader.file_count > 0u && ( reader.entries == nullptr || reader.string_table == nullptr ) ) {
        return pak_error_t::ERR_INVALID_INDEX;
    }

    std::vector<const char *> sorted_paths;
    try {
        sorted_paths.reserve( reader.file_count );
    } catch ( const std::bad_alloc & ) {
        return pak_error_t::ERR_OUT_OF_MEMORY;
    }

    const char *previous_path = nullptr;
    for ( common::u32 i = 0u; i < reader.file_count; ++i ) {
        const pak_disk_file_entry_t &entry = reader.entries[i];
        if ( entry.path_size == 0u || entry.path_size >= CYPHER_PAK_MAX_PATH_LENGTH ) {
            return pak_error_t::ERR_INVALID_INDEX;
        }
        if ( entry.path_offset >= reader.string_table_size ) {
            return pak_error_t::ERR_INVALID_INDEX;
        }

        common::u64 path_end = 0u;
        if ( AddOverflow( entry.path_offset, entry.path_size, path_end ) ||
             path_end >= reader.string_table_size ) {
            return pak_error_t::ERR_INVALID_INDEX;
        }
        if ( reader.string_table[path_end] != '\0' ) {
            return pak_error_t::ERR_INVALID_INDEX;
        }

        const char *path = EntryPath( reader, entry );
        char normalized_path[CYPHER_PAK_MAX_PATH_LENGTH]{};
        const pak_error_t normalize_result = NormalizeVirtualPath( path, normalized_path, sizeof( normalized_path ) );
        if ( normalize_result != pak_error_t::OK ) {
            return normalize_result;
        }
        if ( std::strcmp( path, normalized_path ) != 0 ) {
            return pak_error_t::ERR_INVALID_INDEX;
        }
        if ( entry.path_hash != HashPath32( path ) ) {
            return pak_error_t::ERR_ARCHIVE_CORRUPT;
        }

        const common::u32 entry_flags_allowed = CYPHER_PAK_ENTRY_COMPRESSED | CYPHER_PAK_ENTRY_HAS_HASH;
        if ( ( entry.flags & ~entry_flags_allowed ) != 0u ) {
            return pak_error_t::ERR_UNSUPPORTED_FLAGS;
        }
        if ( entry.compression != static_cast<common::u32>( pak_compression_t::NONE ) ) {
            return pak_error_t::ERR_UNSUPPORTED_COMPRESSION;
        }
        if ( ( entry.flags & CYPHER_PAK_ENTRY_COMPRESSED ) == 0u && entry.stored_size != entry.unpacked_size ) {
            return pak_error_t::ERR_ARCHIVE_CORRUPT;
        }
        if ( !RangeInside( entry.data_offset, entry.stored_size, reader.header.archive_size ) ) {
            return pak_error_t::ERR_ARCHIVE_CORRUPT;
        }
        if ( entry.data_offset < reader.header.data_offset ) {
            return pak_error_t::ERR_ARCHIVE_CORRUPT;
        }

        if ( ( reader.header.flags & CYPHER_PAK_FORMAT_INDEX_SORTED ) != 0u && previous_path != nullptr ) {
            if ( std::strcmp( previous_path, path ) >= 0 ) {
                return pak_error_t::ERR_INVALID_INDEX;
            }
        }
        previous_path = path;
        sorted_paths.push_back( path );
    }

    if ( ( reader.header.flags & CYPHER_PAK_FORMAT_INDEX_SORTED ) == 0u ) {
        std::sort( sorted_paths.begin(), sorted_paths.end(), []( const char *a, const char *b ) {
            return std::strcmp( a, b ) < 0;
        } );
        for ( common::u32 i = 1u; i < static_cast<common::u32>( sorted_paths.size() ); ++i ) {
            if ( std::strcmp( sorted_paths[i - 1u], sorted_paths[i] ) == 0 ) {
                return pak_error_t::ERR_DUPLICATE_ENTRY;
            }
        }
    }

    return pak_error_t::OK;
}

pak_error_t HashFileRange(
    std::FILE *file,
    const common::u64 offset,
    const common::u64 size,
    common::u64 &out_hash )
{
    out_hash = FNV1A64_OFFSET;
    if ( !SeekFile( file, offset ) ) {
        return pak_error_t::ERR_FILE_SEEK_FAILED;
    }

    common::u8 buffer[static_cast<common::usize>( READ_CHUNK_SIZE )]{};
    common::u64 remaining = size;
    while ( remaining > 0u ) {
        const common::u64 chunk64 = remaining < READ_CHUNK_SIZE ? remaining : READ_CHUNK_SIZE;
        const common::usize chunk = static_cast<common::usize>( chunk64 );
        if ( std::fread( buffer, 1u, chunk, file ) != chunk ) {
            return pak_error_t::ERR_FILE_READ_FAILED;
        }
        out_hash = HashBytes64( buffer, chunk64, out_hash );
        remaining -= chunk64;
    }

    return pak_error_t::OK;
}

void FillStats( pak_reader_t &reader )
{
    reader.stats = {};
    reader.stats.file_count = reader.file_count;
    reader.stats.archive_size = reader.header.archive_size;

    for ( common::u32 i = 0u; i < reader.file_count; ++i ) {
        const pak_disk_file_entry_t &entry = reader.entries[i];
        reader.stats.stored_data_size += entry.stored_size;
        reader.stats.unpacked_data_size += entry.unpacked_size;
        if ( entry.compression != static_cast<common::u32>( pak_compression_t::NONE ) ) {
            reader.stats.compressed_file_count++;
        }
    }
}

}       // namespace

pak_error_t CypherPak_ValidateHeader( const pak_header_t &header )
{
    if ( !CypherPak_MagicEquals( header.magic ) ) {
        return pak_error_t::ERR_BAD_MAGIC;
    }
    if ( header.version != CYPHER_PAK_FORMAT_VERSION ) {
        return pak_error_t::ERR_UNSUPPORTED_VERSION;
    }
    if ( header.header_size != CYPHER_PAK_HEADER_SIZE ) {
        return pak_error_t::ERR_INVALID_HEADER;
    }
    if ( header.endian_tag != CYPHER_PAK_ENDIAN_TAG ) {
        return pak_error_t::ERR_ENDIAN_MISMATCH;
    }

    const common::u32 supported_flags =
        CYPHER_PAK_FORMAT_INDEX_SORTED |
        CYPHER_PAK_FORMAT_HAS_FILE_HASHES |
        CYPHER_PAK_FORMAT_HAS_ARCHIVE_HASH;
    if ( ( header.flags & ~supported_flags ) != 0u ) {
        return pak_error_t::ERR_UNSUPPORTED_FLAGS;
    }
    if ( header.file_count > static_cast<common::u64>( std::numeric_limits<common::u32>::max() ) ) {
        return pak_error_t::ERR_INVALID_HEADER;
    }
    if ( header.file_count > std::numeric_limits<common::u64>::max() / CYPHER_PAK_FILE_ENTRY_SIZE ) {
        return pak_error_t::ERR_INVALID_HEADER;
    }

    const common::u64 expected_index_size = header.file_count * CYPHER_PAK_FILE_ENTRY_SIZE;
    if ( header.index_size != expected_index_size ) {
        return pak_error_t::ERR_INVALID_HEADER;
    }

    common::u64 index_end = 0u;
    common::u64 string_end = 0u;
    common::u64 data_end = 0u;
    if ( header.index_offset < header.header_size ) {
        return pak_error_t::ERR_INVALID_HEADER;
    }
    if ( AddOverflow( header.index_offset, header.index_size, index_end ) ) {
        return pak_error_t::ERR_INVALID_HEADER;
    }
    if ( header.string_table_offset < index_end ) {
        return pak_error_t::ERR_INVALID_HEADER;
    }
    if ( AddOverflow( header.string_table_offset, header.string_table_size, string_end ) ) {
        return pak_error_t::ERR_INVALID_HEADER;
    }
    if ( header.data_offset < string_end ) {
        return pak_error_t::ERR_INVALID_HEADER;
    }
    if ( AddOverflow( header.data_offset, header.data_size, data_end ) ) {
        return pak_error_t::ERR_INVALID_HEADER;
    }
    if ( header.archive_size < data_end ) {
        return pak_error_t::ERR_INVALID_HEADER;
    }

    return pak_error_t::OK;
}

pak_error_t CypherPak_OpenReader(
    const char *archive_path,
    const common::u32 flags,
    pak_reader_t &reader )
{
    reader = {};

    if ( archive_path == nullptr || archive_path[0] == '\0' ) {
        return pak_error_t::ERR_INVALID_PATH;
    }
    if ( !OpenFlagsSupported( flags ) ) {
        return pak_error_t::ERR_UNSUPPORTED_FLAGS;
    }
    if ( ( flags & CYPHER_PAK_OPEN_MEMORY_MAP ) != 0u ) {
        return pak_error_t::ERR_NOT_IMPLEMENTED;
    }
    if ( !CopyString( reader.archive_path, sizeof( reader.archive_path ), archive_path ) ) {
        return pak_error_t::ERR_BUFFER_TOO_SMALL;
    }

    std::error_code ec{};
    const common::u64 archive_size = static_cast<common::u64>( std::filesystem::file_size( archive_path, ec ) );
    if ( ec ) {
        return pak_error_t::ERR_FILE_OPEN_FAILED;
    }

    std::FILE *file = std::fopen( archive_path, "rb" );
    if ( file == nullptr ) {
        return pak_error_t::ERR_FILE_OPEN_FAILED;
    }

    common::u8 header_bytes[CYPHER_PAK_HEADER_SIZE]{};
    if ( !ReadExact( file, header_bytes, sizeof( header_bytes ) ) ) {
        std::fclose( file );
        reader = {};
        return pak_error_t::ERR_FILE_READ_FAILED;
    }

    reader.header = DecodeHeader( header_bytes );
    pak_error_t result = CypherPak_ValidateHeader( reader.header );
    if ( result != pak_error_t::OK ) {
        std::fclose( file );
        reader = {};
        return result;
    }
    if ( reader.header.archive_size != archive_size ) {
        std::fclose( file );
        reader = {};
        return pak_error_t::ERR_ARCHIVE_CORRUPT;
    }

    reader.file_count = static_cast<common::u32>( reader.header.file_count );
    if ( reader.file_count > 0u ) {
        reader.entries = new ( std::nothrow ) pak_disk_file_entry_t[reader.file_count];
        if ( reader.entries == nullptr ) {
            std::fclose( file );
            reader = {};
            return pak_error_t::ERR_OUT_OF_MEMORY;
        }
    }

    if ( reader.header.string_table_size > static_cast<common::u64>( std::numeric_limits<common::usize>::max() - 1u ) ) {
        delete[] reader.entries;
        std::fclose( file );
        reader = {};
        return pak_error_t::ERR_OUT_OF_MEMORY;
    }

    reader.string_table_size = reader.header.string_table_size;
    if ( reader.string_table_size > 0u ) {
        reader.string_table = new ( std::nothrow ) char[static_cast<common::usize>( reader.string_table_size ) + 1u];
        if ( reader.string_table == nullptr ) {
            delete[] reader.entries;
            std::fclose( file );
            reader = {};
            return pak_error_t::ERR_OUT_OF_MEMORY;
        }
    }

    if ( reader.file_count > 0u ) {
        if ( !SeekFile( file, reader.header.index_offset ) ) {
            CypherPak_CloseReader( reader );
            std::fclose( file );
            return pak_error_t::ERR_FILE_SEEK_FAILED;
        }

        common::u8 entry_bytes[CYPHER_PAK_FILE_ENTRY_SIZE]{};
        for ( common::u32 i = 0u; i < reader.file_count; ++i ) {
            if ( !ReadExact( file, entry_bytes, sizeof( entry_bytes ) ) ) {
                CypherPak_CloseReader( reader );
                std::fclose( file );
                return pak_error_t::ERR_FILE_READ_FAILED;
            }
            reader.entries[i] = DecodeFileEntry( entry_bytes );
        }
    }

    if ( reader.string_table_size > 0u ) {
        if ( !SeekFile( file, reader.header.string_table_offset ) ) {
            CypherPak_CloseReader( reader );
            std::fclose( file );
            return pak_error_t::ERR_FILE_SEEK_FAILED;
        }
        if ( !ReadExact( file, reader.string_table, reader.string_table_size ) ) {
            CypherPak_CloseReader( reader );
            std::fclose( file );
            return pak_error_t::ERR_FILE_READ_FAILED;
        }
        reader.string_table[reader.string_table_size] = '\0';
    }

    reader.native_file = file;
    reader.flags = flags;
    reader.handle = AllocateReaderHandle();
    reader.open = true;
    FillStats( reader );

    result = ValidateLoadedIndex( reader );
    if ( result != pak_error_t::OK ) {
        CypherPak_CloseReader( reader );
        return result;
    }

    if ( ( flags & CYPHER_PAK_OPEN_VERIFY_FILE_HASHES ) != 0u ) {
        result = CypherPak_Verify( reader, CYPHER_PAK_VERIFY_FILE_HASHES );
        if ( result != pak_error_t::OK ) {
            CypherPak_CloseReader( reader );
            return result;
        }
    }

    return pak_error_t::OK;
}

pak_error_t CypherPak_CloseReader( pak_reader_t &reader )
{
    pak_error_t result = pak_error_t::OK;

    if ( reader.native_file != nullptr ) {
        if ( std::fclose( static_cast<std::FILE *>( reader.native_file ) ) != 0 ) {
            result = pak_error_t::ERR_FILE_CLOSE_FAILED;
        }
    }

    delete[] reader.entries;
    delete[] reader.string_table;
    reader = {};
    return result;
}

bool CypherPak_IsOpen( const pak_reader_t &reader )
{
    return reader.open && reader.native_file != nullptr && reader.handle != CYPHER_PAK_INVALID_HANDLE;
}

pak_error_t CypherPak_GetStats(
    const pak_reader_t &reader,
    pak_stats_t &out_stats )
{
    out_stats = {};

    if ( !CypherPak_IsOpen( reader ) ) {
        return pak_error_t::ERR_INVALID_HANDLE;
    }

    out_stats = reader.stats;
    return pak_error_t::OK;
}

pak_error_t CypherPak_GetFileCount(
    const pak_reader_t &reader,
    common::u32 &out_file_count )
{
    out_file_count = 0u;

    if ( !CypherPak_IsOpen( reader ) ) {
        return pak_error_t::ERR_INVALID_HANDLE;
    }

    out_file_count = reader.file_count;
    return pak_error_t::OK;
}

pak_error_t CypherPak_FindFile(
    const pak_reader_t &reader,
    const char *virtual_path,
    pak_file_index_t &out_index )
{
    out_index = CYPHER_PAK_INVALID_FILE_INDEX;

    if ( !CypherPak_IsOpen( reader ) ) {
        return pak_error_t::ERR_INVALID_HANDLE;
    }

    char normalized_path[CYPHER_PAK_MAX_PATH_LENGTH]{};
    pak_error_t result = NormalizeVirtualPath( virtual_path, normalized_path, sizeof( normalized_path ) );
    if ( result != pak_error_t::OK ) {
        return result;
    }

    if ( ( reader.header.flags & CYPHER_PAK_FORMAT_INDEX_SORTED ) != 0u ) {
        common::u32 left = 0u;
        common::u32 right = reader.file_count;
        while ( left < right ) {
            const common::u32 middle = left + ( right - left ) / 2u;
            const char *entry_path = EntryPath( reader, reader.entries[middle] );
            const int compare = std::strcmp( entry_path, normalized_path );
            if ( compare == 0 ) {
                out_index = middle;
                return pak_error_t::OK;
            }
            if ( compare < 0 ) {
                left = middle + 1u;
            } else {
                right = middle;
            }
        }
        return pak_error_t::ERR_ENTRY_NOT_FOUND;
    }

    const common::u32 path_hash = HashPath32( normalized_path );
    for ( common::u32 i = 0u; i < reader.file_count; ++i ) {
        if ( reader.entries[i].path_hash != path_hash ) {
            continue;
        }
        const char *entry_path = EntryPath( reader, reader.entries[i] );
        if ( entry_path != nullptr && std::strcmp( entry_path, normalized_path ) == 0 ) {
            out_index = i;
            return pak_error_t::OK;
        }
    }

    return pak_error_t::ERR_ENTRY_NOT_FOUND;
}

pak_error_t CypherPak_GetFileInfo(
    const pak_reader_t &reader,
    const pak_file_index_t index,
    pak_file_info_t &out_info )
{
    out_info = {};

    if ( !CypherPak_IsOpen( reader ) ) {
        return pak_error_t::ERR_INVALID_HANDLE;
    }
    if ( index >= reader.file_count ) {
        return pak_error_t::ERR_ENTRY_NOT_FOUND;
    }

    const pak_disk_file_entry_t &entry = reader.entries[index];
    const char *path = EntryPath( reader, entry );
    if ( path == nullptr || !CopyString( out_info.virtual_path, sizeof( out_info.virtual_path ), path ) ) {
        return pak_error_t::ERR_INVALID_INDEX;
    }

    out_info.index = index;
    out_info.data_offset = entry.data_offset;
    out_info.stored_size = entry.stored_size;
    out_info.unpacked_size = entry.unpacked_size;
    out_info.modified_time_utc = entry.modified_time_utc;
    out_info.content_hash = entry.content_hash;
    out_info.path_hash = entry.path_hash;
    out_info.compression = static_cast<pak_compression_t>( entry.compression );
    out_info.flags = entry.flags;

    return pak_error_t::OK;
}

pak_error_t CypherPak_GetFileInfoByPath(
    const pak_reader_t &reader,
    const char *virtual_path,
    pak_file_info_t &out_info )
{
    out_info = {};

    pak_file_index_t index = CYPHER_PAK_INVALID_FILE_INDEX;
    const pak_error_t result = CypherPak_FindFile( reader, virtual_path, index );
    if ( result != pak_error_t::OK ) {
        return result;
    }

    return CypherPak_GetFileInfo( reader, index, out_info );
}

pak_error_t CypherPak_ReadRawFileByIndex(
    pak_reader_t &reader,
    const pak_file_index_t index,
    void *buffer,
    const common::u64 buffer_size,
    common::u64 &out_bytes_read )
{
    out_bytes_read = 0u;

    if ( !CypherPak_IsOpen( reader ) ) {
        return pak_error_t::ERR_INVALID_HANDLE;
    }
    if ( index >= reader.file_count ) {
        return pak_error_t::ERR_ENTRY_NOT_FOUND;
    }

    const pak_disk_file_entry_t &entry = reader.entries[index];
    if ( entry.stored_size > 0u && buffer == nullptr ) {
        return pak_error_t::ERR_INVALID_ARGUMENT;
    }
    if ( buffer_size < entry.stored_size ) {
        return pak_error_t::ERR_BUFFER_TOO_SMALL;
    }

    std::FILE *file = static_cast<std::FILE *>( reader.native_file );
    if ( !SeekFile( file, entry.data_offset ) ) {
        return pak_error_t::ERR_FILE_SEEK_FAILED;
    }
    if ( !ReadExact( file, buffer, entry.stored_size ) ) {
        return pak_error_t::ERR_FILE_READ_FAILED;
    }

    out_bytes_read = entry.stored_size;
    reader.stats.read_count++;
    reader.stats.bytes_read += out_bytes_read;
    return pak_error_t::OK;
}

pak_error_t CypherPak_ReadFileByIndex(
    pak_reader_t &reader,
    const pak_file_index_t index,
    void *buffer,
    const common::u64 buffer_size,
    common::u64 &out_bytes_read )
{
    out_bytes_read = 0u;

    if ( !CypherPak_IsOpen( reader ) ) {
        return pak_error_t::ERR_INVALID_HANDLE;
    }
    if ( index >= reader.file_count ) {
        return pak_error_t::ERR_ENTRY_NOT_FOUND;
    }

    const pak_disk_file_entry_t &entry = reader.entries[index];
    if ( entry.compression != static_cast<common::u32>( pak_compression_t::NONE ) ) {
        return pak_error_t::ERR_UNSUPPORTED_COMPRESSION;
    }
    if ( buffer_size < entry.unpacked_size ) {
        return pak_error_t::ERR_BUFFER_TOO_SMALL;
    }

    return CypherPak_ReadRawFileByIndex( reader, index, buffer, buffer_size, out_bytes_read );
}

pak_error_t CypherPak_ReadFile(
    pak_reader_t &reader,
    const char *virtual_path,
    void *buffer,
    const common::u64 buffer_size,
    common::u64 &out_bytes_read )
{
    out_bytes_read = 0u;

    pak_file_index_t index = CYPHER_PAK_INVALID_FILE_INDEX;
    const pak_error_t result = CypherPak_FindFile( reader, virtual_path, index );
    if ( result != pak_error_t::OK ) {
        return result;
    }

    return CypherPak_ReadFileByIndex( reader, index, buffer, buffer_size, out_bytes_read );
}

pak_error_t CypherPak_Verify(
    pak_reader_t &reader,
    const common::u32 flags )
{
    if ( !CypherPak_IsOpen( reader ) ) {
        return pak_error_t::ERR_INVALID_HANDLE;
    }
    if ( flags == CYPHER_PAK_VERIFY_NONE ) {
        return pak_error_t::OK;
    }

    if ( ( flags & CYPHER_PAK_VERIFY_HEADER ) != 0u ) {
        const pak_error_t header_result = CypherPak_ValidateHeader( reader.header );
        if ( header_result != pak_error_t::OK ) {
            return header_result;
        }
    }
    if ( ( flags & CYPHER_PAK_VERIFY_INDEX ) != 0u ) {
        const pak_error_t index_result = ValidateLoadedIndex( reader );
        if ( index_result != pak_error_t::OK ) {
            return index_result;
        }
    }
    if ( ( flags & CYPHER_PAK_VERIFY_FILE_HASHES ) != 0u ) {
        for ( common::u32 i = 0u; i < reader.file_count; ++i ) {
            const pak_disk_file_entry_t &entry = reader.entries[i];
            if ( ( entry.flags & CYPHER_PAK_ENTRY_HAS_HASH ) == 0u ) {
                continue;
            }

            common::u64 hash = 0u;
            const pak_error_t hash_result = HashFileRange(
                static_cast<std::FILE *>( reader.native_file ),
                entry.data_offset,
                entry.stored_size,
                hash );
            if ( hash_result != pak_error_t::OK ) {
                return hash_result;
            }
            if ( hash != entry.content_hash ) {
                return pak_error_t::ERR_CHECKSUM_MISMATCH;
            }
        }
    }
    if ( ( flags & CYPHER_PAK_VERIFY_ARCHIVE_HASH ) != 0u &&
         ( reader.header.flags & CYPHER_PAK_FORMAT_HAS_ARCHIVE_HASH ) != 0u ) {
        return pak_error_t::ERR_NOT_IMPLEMENTED;
    }

    return pak_error_t::OK;
}

}       // namespace cypher::engine::pak

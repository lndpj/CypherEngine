#include "CypherEngine/CypherPak/CypherPak_Writer.h"
#include "CypherEngine/CypherPak/CypherPak_Compression.h"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <limits>
#include <new>
#include <string>
#include <vector>

namespace cypher::engine::pak
{

namespace {

constexpr common::u32 FNV1A32_OFFSET = 2166136261u;
constexpr common::u32 FNV1A32_PRIME = 16777619u;
constexpr common::u64 FNV1A64_OFFSET = 14695981039346656037ull;
constexpr common::u64 FNV1A64_PRIME = 1099511628211ull;
constexpr common::u64 IO_CHUNK_SIZE = 64u * 1024u;

std::atomic<common::u32> g_next_writer_handle{ 1u };

struct source_file_copy_t {
    std::string virtual_path;
    std::string physical_path;
    pak_compression_t compression{ pak_compression_t::NONE };
    common::u32 flags{ CYPHER_PAK_ENTRY_NONE };
};

struct packed_file_t {
    std::string virtual_path;
    std::string physical_path;
    std::vector<common::u8> data;
    pak_compression_t compression{ pak_compression_t::NONE };
    common::u32 flags{ CYPHER_PAK_ENTRY_NONE };
    common::u64 path_offset{ 0u };
    common::u64 data_offset{ 0u };
    common::u64 stored_size{ 0u };
    common::u64 unpacked_size{ 0u };
    common::u64 modified_time_utc{ 0u };
    common::u64 content_hash{ 0u };
    common::u32 path_hash{ 0u };
};

struct writer_state_t {
    std::vector<source_file_copy_t> files;
};

pak_handle_t AllocateWriterHandle()
{
    common::u32 handle = g_next_writer_handle.fetch_add( 1u );
    if ( handle == CYPHER_PAK_INVALID_HANDLE ) {
        handle = g_next_writer_handle.fetch_add( 1u );
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

bool WriterFlagsSupported( const common::u32 flags )
{
    const common::u32 supported_flags =
        CYPHER_PAK_WRITER_DETERMINISTIC |
        CYPHER_PAK_WRITER_SORT_INDEX |
        CYPHER_PAK_WRITER_WRITE_HASHES |
        CYPHER_PAK_WRITER_FAIL_ON_DUPLICATE;
    return ( flags & ~supported_flags ) == 0u;
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
        const common::u64 chunk64 = remaining < IO_CHUNK_SIZE ? remaining : IO_CHUNK_SIZE;
        const common::usize chunk = static_cast<common::usize>( chunk64 );
        if ( std::fread( write, 1u, chunk, file ) != chunk ) {
            return false;
        }
        write += chunk;
        remaining -= chunk64;
    }

    return true;
}

bool WriteExact( std::FILE *file, const void *buffer, const common::u64 size )
{
    if ( size == 0u ) {
        return true;
    }
    if ( file == nullptr || buffer == nullptr ) {
        return false;
    }

    const common::u8 *read = static_cast<const common::u8 *>( buffer );
    common::u64 remaining = size;
    while ( remaining > 0u ) {
        const common::u64 chunk64 = remaining < IO_CHUNK_SIZE ? remaining : IO_CHUNK_SIZE;
        const common::usize chunk = static_cast<common::usize>( chunk64 );
        if ( std::fwrite( read, 1u, chunk, file ) != chunk ) {
            return false;
        }
        read += chunk;
        remaining -= chunk64;
    }

    return true;
}

pak_error_t WriteZeros( std::FILE *file, const common::u64 count )
{
    common::u8 zeros[4096]{};
    common::u64 remaining = count;

    while ( remaining > 0u ) {
        const common::u64 chunk64 = remaining < sizeof( zeros ) ? remaining : sizeof( zeros );
        if ( !WriteExact( file, zeros, chunk64 ) ) {
            return pak_error_t::ERR_FILE_WRITE_FAILED;
        }
        remaining -= chunk64;
    }

    return pak_error_t::OK;
}

common::u64 FileModifiedTimeUtc( const std::filesystem::path &path )
{
    std::error_code ec{};
    const auto file_time = std::filesystem::last_write_time( path, ec );
    if ( ec ) {
        return 0u;
    }

    const auto system_time = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
        file_time - std::filesystem::file_time_type::clock::now() + std::chrono::system_clock::now() );
    const std::time_t time = std::chrono::system_clock::to_time_t( system_time );
    return time < 0 ? 0u : static_cast<common::u64>( time );
}

pak_error_t ReadPhysicalFile(
    const char *physical_path,
    std::vector<common::u8> &out_data,
    common::u64 &out_modified_time )
{
    out_data.clear();
    out_modified_time = 0u;

    if ( physical_path == nullptr || physical_path[0] == '\0' ) {
        return pak_error_t::ERR_INVALID_PATH;
    }

    std::error_code ec{};
    const std::filesystem::path path( physical_path );
    if ( !std::filesystem::exists( path, ec ) || ec ) {
        return ec ? pak_error_t::ERR_IO_ERROR : pak_error_t::ERR_PATH_NOT_FOUND;
    }
    if ( !std::filesystem::is_regular_file( path, ec ) || ec ) {
        return ec ? pak_error_t::ERR_IO_ERROR : pak_error_t::ERR_INVALID_PATH;
    }

    const common::u64 file_size = static_cast<common::u64>( std::filesystem::file_size( path, ec ) );
    if ( ec ) {
        return pak_error_t::ERR_IO_ERROR;
    }
    if ( file_size > static_cast<common::u64>( std::numeric_limits<common::usize>::max() ) ) {
        return pak_error_t::ERR_OUT_OF_MEMORY;
    }

    try {
        out_data.resize( static_cast<common::usize>( file_size ) );
    } catch ( const std::bad_alloc & ) {
        return pak_error_t::ERR_OUT_OF_MEMORY;
    }

    std::FILE *file = std::fopen( physical_path, "rb" );
    if ( file == nullptr ) {
        out_data.clear();
        return pak_error_t::ERR_FILE_OPEN_FAILED;
    }

    const bool read_ok = ReadExact( file, out_data.data(), file_size );
    const bool close_ok = std::fclose( file ) == 0;
    if ( !read_ok || !close_ok ) {
        out_data.clear();
        return read_ok ? pak_error_t::ERR_FILE_CLOSE_FAILED : pak_error_t::ERR_FILE_READ_FAILED;
    }

    out_modified_time = FileModifiedTimeUtc( path );
    return pak_error_t::OK;
}

void EncodeHeader( const pak_header_t &header, common::u8 *bytes )
{
    common::usize offset = 0u;

    std::memcpy( bytes + offset, header.magic, CYPHER_PAK_MAGIC_SIZE );
    offset += CYPHER_PAK_MAGIC_SIZE;

    CypherPak_StoreU32LE( bytes + offset, header.version );
    offset += 4u;
    CypherPak_StoreU32LE( bytes + offset, header.header_size );
    offset += 4u;
    CypherPak_StoreU32LE( bytes + offset, header.endian_tag );
    offset += 4u;
    CypherPak_StoreU32LE( bytes + offset, header.flags );
    offset += 4u;

    CypherPak_StoreU64LE( bytes + offset, header.archive_size );
    offset += 8u;
    CypherPak_StoreU64LE( bytes + offset, header.file_count );
    offset += 8u;
    CypherPak_StoreU64LE( bytes + offset, header.index_offset );
    offset += 8u;
    CypherPak_StoreU64LE( bytes + offset, header.index_size );
    offset += 8u;
    CypherPak_StoreU64LE( bytes + offset, header.string_table_offset );
    offset += 8u;
    CypherPak_StoreU64LE( bytes + offset, header.string_table_size );
    offset += 8u;
    CypherPak_StoreU64LE( bytes + offset, header.data_offset );
    offset += 8u;
    CypherPak_StoreU64LE( bytes + offset, header.data_size );
    offset += 8u;
    CypherPak_StoreU64LE( bytes + offset, header.archive_hash );
    offset += 8u;

    for ( common::u32 i = 0u; i < 4u; ++i ) {
        CypherPak_StoreU64LE( bytes + offset, header.reserved[i] );
        offset += 8u;
    }
}

void EncodeFileEntry( const pak_disk_file_entry_t &entry, common::u8 *bytes )
{
    common::usize offset = 0u;

    CypherPak_StoreU64LE( bytes + offset, entry.path_offset );
    offset += 8u;
    CypherPak_StoreU64LE( bytes + offset, entry.data_offset );
    offset += 8u;
    CypherPak_StoreU64LE( bytes + offset, entry.stored_size );
    offset += 8u;
    CypherPak_StoreU64LE( bytes + offset, entry.unpacked_size );
    offset += 8u;
    CypherPak_StoreU64LE( bytes + offset, entry.modified_time_utc );
    offset += 8u;
    CypherPak_StoreU64LE( bytes + offset, entry.content_hash );
    offset += 8u;
    CypherPak_StoreU32LE( bytes + offset, entry.path_size );
    offset += 4u;
    CypherPak_StoreU32LE( bytes + offset, entry.path_hash );
    offset += 4u;
    CypherPak_StoreU32LE( bytes + offset, entry.compression );
    offset += 4u;
    CypherPak_StoreU32LE( bytes + offset, entry.flags );
}

pak_error_t MakeSourceCopy( const pak_source_file_t &source, source_file_copy_t &out_copy )
{
    out_copy = {};

    if ( source.virtual_path == nullptr || source.virtual_path[0] == '\0' ||
         source.physical_path == nullptr || source.physical_path[0] == '\0' ) {
        return pak_error_t::ERR_INVALID_ARGUMENT;
    }

    const common::u32 allowed_entry_flags = CYPHER_PAK_ENTRY_HAS_HASH;
    if ( ( source.flags & ~allowed_entry_flags ) != 0u ) {
        return pak_error_t::ERR_UNSUPPORTED_FLAGS;
    }
    if ( !CypherPak_CompressionSupported( source.compression ) ) {
        return pak_error_t::ERR_UNSUPPORTED_COMPRESSION;
    }

    try {
        out_copy.virtual_path = source.virtual_path;
        out_copy.physical_path = source.physical_path;
    } catch ( const std::bad_alloc & ) {
        return pak_error_t::ERR_OUT_OF_MEMORY;
    }
    out_copy.compression = source.compression;
    out_copy.flags = source.flags;
    return pak_error_t::OK;
}

pak_error_t PrepareFiles(
    const std::vector<source_file_copy_t> &source_files,
    const common::u32 writer_flags,
    std::vector<packed_file_t> &out_files )
{
    out_files.clear();

    try {
        out_files.reserve( source_files.size() );
    } catch ( const std::bad_alloc & ) {
        return pak_error_t::ERR_OUT_OF_MEMORY;
    }

    for ( const source_file_copy_t &source : source_files ) {
        packed_file_t file{};

        char normalized_path[CYPHER_PAK_MAX_PATH_LENGTH]{};
        pak_error_t result = NormalizeVirtualPath( source.virtual_path.c_str(), normalized_path, sizeof( normalized_path ) );
        if ( result != pak_error_t::OK ) {
            return result;
        }

        try {
            file.virtual_path = normalized_path;
            file.physical_path = source.physical_path;
        } catch ( const std::bad_alloc & ) {
            return pak_error_t::ERR_OUT_OF_MEMORY;
        }

        file.compression = source.compression;
        file.flags = source.flags;
        if ( ( writer_flags & CYPHER_PAK_WRITER_WRITE_HASHES ) != 0u ) {
            file.flags |= CYPHER_PAK_ENTRY_HAS_HASH;
        }
        if ( file.compression != pak_compression_t::NONE ) {
            file.flags |= CYPHER_PAK_ENTRY_COMPRESSED;
        }

        result = ReadPhysicalFile( source.physical_path.c_str(), file.data, file.modified_time_utc );
        if ( result != pak_error_t::OK ) {
            return result;
        }

        file.stored_size = static_cast<common::u64>( file.data.size() );
        file.unpacked_size = file.stored_size;
        file.path_hash = HashPath32( file.virtual_path.c_str() );
        file.content_hash = HashBytes64( file.data.data(), file.stored_size );

        try {
            out_files.push_back( std::move( file ) );
        } catch ( const std::bad_alloc & ) {
            return pak_error_t::ERR_OUT_OF_MEMORY;
        }
    }

    if ( ( writer_flags & CYPHER_PAK_WRITER_SORT_INDEX ) != 0u ) {
        std::sort( out_files.begin(), out_files.end(), []( const packed_file_t &a, const packed_file_t &b ) {
            return a.virtual_path < b.virtual_path;
        } );
    }

    std::vector<std::string> sorted_paths;
    try {
        sorted_paths.reserve( out_files.size() );
        for ( const packed_file_t &file : out_files ) {
            sorted_paths.push_back( file.virtual_path );
        }
    } catch ( const std::bad_alloc & ) {
        return pak_error_t::ERR_OUT_OF_MEMORY;
    }

    std::sort( sorted_paths.begin(), sorted_paths.end() );
    for ( common::usize i = 1u; i < sorted_paths.size(); ++i ) {
        if ( sorted_paths[i - 1u] == sorted_paths[i] ) {
            return pak_error_t::ERR_DUPLICATE_ENTRY;
        }
    }

    return pak_error_t::OK;
}

pak_error_t BuildStringTable(
    std::vector<packed_file_t> &files,
    std::vector<char> &out_string_table )
{
    out_string_table.clear();

    for ( packed_file_t &file : files ) {
        if ( out_string_table.size() > std::numeric_limits<common::u64>::max() ) {
            return pak_error_t::ERR_INTEGER_OVERFLOW;
        }

        file.path_offset = static_cast<common::u64>( out_string_table.size() );
        try {
            out_string_table.insert( out_string_table.end(), file.virtual_path.begin(), file.virtual_path.end() );
            out_string_table.push_back( '\0' );
        } catch ( const std::bad_alloc & ) {
            return pak_error_t::ERR_OUT_OF_MEMORY;
        }
    }

    return pak_error_t::OK;
}

pak_error_t BuildArchive(
    const char *archive_path,
    const common::u32 writer_flags,
    const pak_compression_t default_compression,
    const common::u32 data_alignment,
    const std::vector<source_file_copy_t> &source_files )
{
    if ( archive_path == nullptr || archive_path[0] == '\0' ) {
        return pak_error_t::ERR_INVALID_PATH;
    }
    if ( !WriterFlagsSupported( writer_flags ) ) {
        return pak_error_t::ERR_UNSUPPORTED_FLAGS;
    }
    if ( !CypherPak_CompressionSupported( default_compression ) ) {
        return pak_error_t::ERR_UNSUPPORTED_COMPRESSION;
    }
    if ( source_files.size() > static_cast<common::usize>( std::numeric_limits<common::u32>::max() ) ) {
        return pak_error_t::ERR_INTEGER_OVERFLOW;
    }

    std::vector<packed_file_t> files;
    pak_error_t result = PrepareFiles( source_files, writer_flags, files );
    if ( result != pak_error_t::OK ) {
        return result;
    }

    std::vector<char> string_table;
    result = BuildStringTable( files, string_table );
    if ( result != pak_error_t::OK ) {
        return result;
    }

    const common::u64 file_count = static_cast<common::u64>( files.size() );
    if ( file_count > std::numeric_limits<common::u64>::max() / CYPHER_PAK_FILE_ENTRY_SIZE ) {
        return pak_error_t::ERR_INTEGER_OVERFLOW;
    }

    const common::u64 alignment = data_alignment == 0u ? 1u : data_alignment;
    pak_header_t header{};
    std::memcpy( header.magic, CYPHER_PAK_MAGIC, CYPHER_PAK_MAGIC_SIZE );
    header.version = CYPHER_PAK_FORMAT_VERSION;
    header.header_size = CYPHER_PAK_HEADER_SIZE;
    header.endian_tag = CYPHER_PAK_ENDIAN_TAG;
    header.flags = CYPHER_PAK_FORMAT_NONE;
    if ( ( writer_flags & CYPHER_PAK_WRITER_SORT_INDEX ) != 0u ) {
        header.flags |= CYPHER_PAK_FORMAT_INDEX_SORTED;
    }
    if ( ( writer_flags & CYPHER_PAK_WRITER_WRITE_HASHES ) != 0u ) {
        header.flags |= CYPHER_PAK_FORMAT_HAS_FILE_HASHES;
    }

    header.file_count = file_count;
    header.index_offset = CYPHER_PAK_HEADER_SIZE;
    header.index_size = file_count * CYPHER_PAK_FILE_ENTRY_SIZE;

    if ( AddOverflow( header.index_offset, header.index_size, header.string_table_offset ) ) {
        return pak_error_t::ERR_INTEGER_OVERFLOW;
    }

    header.string_table_size = static_cast<common::u64>( string_table.size() );
    common::u64 string_table_end = 0u;
    if ( AddOverflow( header.string_table_offset, header.string_table_size, string_table_end ) ) {
        return pak_error_t::ERR_INTEGER_OVERFLOW;
    }
    header.data_offset = CypherPak_AlignUp64( string_table_end, alignment );

    common::u64 cursor = header.data_offset;
    for ( packed_file_t &file : files ) {
        cursor = CypherPak_AlignUp64( cursor, alignment );
        file.data_offset = cursor;
        if ( AddOverflow( cursor, file.stored_size, cursor ) ) {
            return pak_error_t::ERR_INTEGER_OVERFLOW;
        }
    }

    header.data_size = cursor >= header.data_offset ? cursor - header.data_offset : 0u;
    header.archive_size = cursor;

    std::error_code ec{};
    const std::filesystem::path output_path( archive_path );
    const std::filesystem::path parent_path = output_path.parent_path();
    if ( !parent_path.empty() ) {
        std::filesystem::create_directories( parent_path, ec );
        if ( ec ) {
            return pak_error_t::ERR_IO_ERROR;
        }
    }

    std::FILE *archive = std::fopen( archive_path, "wb" );
    if ( archive == nullptr ) {
        return pak_error_t::ERR_FILE_OPEN_FAILED;
    }

    auto fail = [&]( const pak_error_t error ) {
        std::fclose( archive );
        std::filesystem::remove( output_path, ec );
        return error;
    };

    common::u8 header_bytes[CYPHER_PAK_HEADER_SIZE]{};
    EncodeHeader( header, header_bytes );
    if ( !WriteExact( archive, header_bytes, sizeof( header_bytes ) ) ) {
        return fail( pak_error_t::ERR_FILE_WRITE_FAILED );
    }

    common::u8 entry_bytes[CYPHER_PAK_FILE_ENTRY_SIZE]{};
    for ( const packed_file_t &file : files ) {
        pak_disk_file_entry_t disk_entry{};
        disk_entry.path_offset = file.path_offset;
        disk_entry.data_offset = file.data_offset;
        disk_entry.stored_size = file.stored_size;
        disk_entry.unpacked_size = file.unpacked_size;
        disk_entry.modified_time_utc = file.modified_time_utc;
        disk_entry.content_hash = file.content_hash;
        disk_entry.path_size = static_cast<common::u32>( file.virtual_path.size() );
        disk_entry.path_hash = file.path_hash;
        disk_entry.compression = static_cast<common::u32>( file.compression );
        disk_entry.flags = file.flags;

        EncodeFileEntry( disk_entry, entry_bytes );
        if ( !WriteExact( archive, entry_bytes, sizeof( entry_bytes ) ) ) {
            return fail( pak_error_t::ERR_FILE_WRITE_FAILED );
        }
    }

    if ( !WriteExact( archive, string_table.data(), string_table.size() ) ) {
        return fail( pak_error_t::ERR_FILE_WRITE_FAILED );
    }

    common::u64 written_offset = header.string_table_offset + header.string_table_size;
    if ( header.data_offset > written_offset ) {
        result = WriteZeros( archive, header.data_offset - written_offset );
        if ( result != pak_error_t::OK ) {
            return fail( result );
        }
        written_offset = header.data_offset;
    }

    for ( const packed_file_t &file : files ) {
        if ( file.data_offset > written_offset ) {
            result = WriteZeros( archive, file.data_offset - written_offset );
            if ( result != pak_error_t::OK ) {
                return fail( result );
            }
            written_offset = file.data_offset;
        }
        if ( !WriteExact( archive, file.data.data(), file.stored_size ) ) {
            return fail( pak_error_t::ERR_FILE_WRITE_FAILED );
        }
        written_offset += file.stored_size;
    }

    if ( written_offset < header.archive_size ) {
        result = WriteZeros( archive, header.archive_size - written_offset );
        if ( result != pak_error_t::OK ) {
            return fail( result );
        }
    }

    if ( std::fclose( archive ) != 0 ) {
        std::filesystem::remove( output_path, ec );
        return pak_error_t::ERR_FILE_CLOSE_FAILED;
    }

    return pak_error_t::OK;
}

}       // namespace

pak_error_t CypherPak_CreateArchive(
    const pak_writer_config_t &config,
    const pak_source_file_t *files,
    const common::u32 file_count )
{
    if ( file_count > 0u && files == nullptr ) {
        return pak_error_t::ERR_INVALID_ARGUMENT;
    }

    std::vector<source_file_copy_t> source_files;
    try {
        source_files.reserve( file_count );
    } catch ( const std::bad_alloc & ) {
        return pak_error_t::ERR_OUT_OF_MEMORY;
    }

    for ( common::u32 i = 0u; i < file_count; ++i ) {
        source_file_copy_t copy{};
        const pak_error_t result = MakeSourceCopy( files[i], copy );
        if ( result != pak_error_t::OK ) {
            return result;
        }
        try {
            source_files.push_back( std::move( copy ) );
        } catch ( const std::bad_alloc & ) {
            return pak_error_t::ERR_OUT_OF_MEMORY;
        }
    }

    return BuildArchive(
        config.archive_path,
        config.flags,
        config.default_compression,
        config.data_alignment,
        source_files );
}

pak_error_t CypherPak_BeginWriter(
    const pak_writer_config_t &config,
    pak_writer_t &writer )
{
    writer = {};

    if ( config.archive_path == nullptr || config.archive_path[0] == '\0' ) {
        return pak_error_t::ERR_INVALID_PATH;
    }
    if ( !WriterFlagsSupported( config.flags ) ) {
        return pak_error_t::ERR_UNSUPPORTED_FLAGS;
    }
    if ( !CypherPak_CompressionSupported( config.default_compression ) ) {
        return pak_error_t::ERR_UNSUPPORTED_COMPRESSION;
    }
    if ( !CopyString( writer.archive_path, sizeof( writer.archive_path ), config.archive_path ) ) {
        return pak_error_t::ERR_BUFFER_TOO_SMALL;
    }

    writer_state_t *state = new ( std::nothrow ) writer_state_t();
    if ( state == nullptr ) {
        return pak_error_t::ERR_OUT_OF_MEMORY;
    }

    writer.handle = AllocateWriterHandle();
    writer.builder_state = state;
    writer.flags = config.flags;
    writer.default_compression = config.default_compression;
    writer.data_alignment = config.data_alignment;
    writer.open = true;
    writer.finalized = false;
    return pak_error_t::OK;
}

pak_error_t CypherPak_AddFile(
    pak_writer_t &writer,
    const pak_source_file_t &file )
{
    if ( !writer.open || writer.builder_state == nullptr || writer.handle == CYPHER_PAK_INVALID_HANDLE ) {
        return pak_error_t::ERR_INVALID_HANDLE;
    }
    if ( writer.finalized ) {
        return pak_error_t::ERR_INVALID_STATE;
    }

    source_file_copy_t copy{};
    pak_error_t result = MakeSourceCopy( file, copy );
    if ( result != pak_error_t::OK ) {
        return result;
    }

    writer_state_t *state = static_cast<writer_state_t *>( writer.builder_state );
    try {
        state->files.push_back( std::move( copy ) );
    } catch ( const std::bad_alloc & ) {
        return pak_error_t::ERR_OUT_OF_MEMORY;
    }

    writer.file_count = static_cast<common::u32>( state->files.size() );
    return pak_error_t::OK;
}

pak_error_t CypherPak_FinishWriter( pak_writer_t &writer )
{
    if ( !writer.open || writer.builder_state == nullptr || writer.handle == CYPHER_PAK_INVALID_HANDLE ) {
        return pak_error_t::ERR_INVALID_HANDLE;
    }
    if ( writer.finalized ) {
        return pak_error_t::ERR_INVALID_STATE;
    }

    writer_state_t *state = static_cast<writer_state_t *>( writer.builder_state );
    const pak_error_t result = BuildArchive(
        writer.archive_path,
        writer.flags,
        writer.default_compression,
        writer.data_alignment,
        state->files );

    delete state;
    writer.builder_state = nullptr;
    writer.native_file = nullptr;
    writer.open = false;
    writer.finalized = result == pak_error_t::OK;
    writer.handle = CYPHER_PAK_INVALID_HANDLE;
    return result;
}

pak_error_t CypherPak_CancelWriter( pak_writer_t &writer )
{
    if ( writer.builder_state != nullptr ) {
        delete static_cast<writer_state_t *>( writer.builder_state );
    }
    writer = {};
    return pak_error_t::OK;
}

}       // namespace cypher::engine::pak

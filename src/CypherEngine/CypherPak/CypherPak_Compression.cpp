#include "CypherEngine/CypherPak/CypherPak_Compression.h"

#include <cstring>

namespace cypher::engine::pak
{

const char *CypherPak_CompressionName( const pak_compression_t method )
{
    switch ( method ) {
    case pak_compression_t::NONE:
        return "NONE";
    case pak_compression_t::LZ4:
        return "LZ4";
    case pak_compression_t::ZSTD:
        return "ZSTD";
    default:
        return "UNKNOWN";
    }
}

bool CypherPak_CompressionSupported( const pak_compression_t method )
{
    return ( method == pak_compression_t::NONE );
}

pak_error_t CypherPak_CompressBound(
    const pak_compression_t method,
    const common::u64 input_size,
    common::u64 &out_max_output_size )
{
    out_max_output_size = 0u;

    if ( !CypherPak_CompressionSupported( method ) ) {
        return pak_error_t::ERR_UNSUPPORTED_COMPRESSION;
    }

    out_max_output_size = input_size;
    return pak_error_t::OK;
}

pak_error_t CypherPak_Compress(
    const pak_compression_config_t &config,
    const void *input,
    const common::u64 input_size,
    void *output,
    const common::u64 output_size,
    common::u64 &out_bytes_written )
{
    out_bytes_written = 0u;

    if ( !CypherPak_CompressionSupported( config.method ) ) {
        return pak_error_t::ERR_UNSUPPORTED_COMPRESSION;
    }
    if ( input_size > 0u && input == nullptr ) {
        return pak_error_t::ERR_INVALID_ARGUMENT;
    }
    if ( input_size > 0u && output == nullptr ) {
        return pak_error_t::ERR_INVALID_ARGUMENT;
    }
    if ( output_size < input_size ) {
        return pak_error_t::ERR_BUFFER_TOO_SMALL;
    }

    if ( input_size > 0u ) {
        std::memcpy( output, input, static_cast<common::usize>( input_size ) );
    }
    out_bytes_written = input_size;
    return pak_error_t::OK;
}

pak_error_t CypherPak_Decompress(
    const pak_compression_t method,
    const void *input,
    const common::u64 input_size,
    void *output,
    const common::u64 output_size,
    common::u64 &out_bytes_written )
{
    out_bytes_written = 0u;

    if ( !CypherPak_CompressionSupported( method ) ) {
        return pak_error_t::ERR_UNSUPPORTED_COMPRESSION;
    }
    if ( input_size > 0u && input == nullptr ) {
        return pak_error_t::ERR_INVALID_ARGUMENT;
    }
    if ( input_size > 0u && output == nullptr ) {
        return pak_error_t::ERR_INVALID_ARGUMENT;
    }
    if ( output_size < input_size ) {
        return pak_error_t::ERR_BUFFER_TOO_SMALL;
    }

    if ( input_size > 0u ) {
        std::memcpy( output, input, static_cast<common::usize>( input_size ) );
    }
    out_bytes_written = input_size;
    return pak_error_t::OK;
}

}       // namespace cypher::engine::pak

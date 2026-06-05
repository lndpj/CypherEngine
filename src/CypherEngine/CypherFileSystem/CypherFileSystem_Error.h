#pragma once

#include "CypherEngine/CypherCommon/CypherCommon_Error.h"

namespace cypher::engine::fs
{

/*
================
Filesystem Error Codes
================
*/
enum class cypher_filesystem_error_code_t : common::u8 {
    OK = 0,

    ERR_NOT_INIT,
    ERR_IS_INIT,

    ERR_INVALID_PATH,
    ERR_INVALID_MODE,
    ERR_INVALID_HANDLE,
    ERR_INVALID_ARGUMENT,

    ERR_TOO_MANY_MOUNTS,
    ERR_MOUNT_NOT_FOUND,
    ERR_PATH_NOT_FOUND,

    ERR_FILE_OPEN_FAILED,
    ERR_FILE_CLOSE_FAILED,
    ERR_FILE_READ_FAILED,
    ERR_FILE_WRITE_FAILED,
    ERR_FILE_SEEK_FAILED,
    ERR_FILE_TELL_FAILED,

    ERR_BUFFER_TOO_SMALL,
    ERR_UNSUPPORTED_BACKEND,
    ERR_PERMISSION_DENIED,
    ERR_IO_ERROR
};

/*
================
Filesystem Error Helpers
================
*/
constexpr inline const char *CypherFileSystem_ErrorName( const cypher_filesystem_error_code_t error ) {
    switch ( error ) {
    case cypher_filesystem_error_code_t::OK:
        return "OK";
    case cypher_filesystem_error_code_t::ERR_NOT_INIT:
        return "ERR_NOT_INIT";
    case cypher_filesystem_error_code_t::ERR_IS_INIT:
        return "ERR_IS_INIT";
    case cypher_filesystem_error_code_t::ERR_INVALID_PATH:
        return "ERR_INVALID_PATH";
    case cypher_filesystem_error_code_t::ERR_INVALID_MODE:
        return "ERR_INVALID_MODE";
    case cypher_filesystem_error_code_t::ERR_INVALID_HANDLE:
        return "ERR_INVALID_HANDLE";
    case cypher_filesystem_error_code_t::ERR_INVALID_ARGUMENT:
        return "ERR_INVALID_ARGUMENT";
    case cypher_filesystem_error_code_t::ERR_TOO_MANY_MOUNTS:
        return "ERR_TOO_MANY_MOUNTS";
    case cypher_filesystem_error_code_t::ERR_MOUNT_NOT_FOUND:
        return "ERR_MOUNT_NOT_FOUND";
    case cypher_filesystem_error_code_t::ERR_PATH_NOT_FOUND:
        return "ERR_PATH_NOT_FOUND";
    case cypher_filesystem_error_code_t::ERR_FILE_OPEN_FAILED:
        return "ERR_FILE_OPEN_FAILED";
    case cypher_filesystem_error_code_t::ERR_FILE_CLOSE_FAILED:
        return "ERR_FILE_CLOSE_FAILED";
    case cypher_filesystem_error_code_t::ERR_FILE_READ_FAILED:
        return "ERR_FILE_READ_FAILED";
    case cypher_filesystem_error_code_t::ERR_FILE_WRITE_FAILED:
        return "ERR_FILE_WRITE_FAILED";
    case cypher_filesystem_error_code_t::ERR_FILE_SEEK_FAILED:
        return "ERR_FILE_SEEK_FAILED";
    case cypher_filesystem_error_code_t::ERR_FILE_TELL_FAILED:
        return "ERR_FILE_TELL_FAILED";
    case cypher_filesystem_error_code_t::ERR_BUFFER_TOO_SMALL:
        return "ERR_BUFFER_TOO_SMALL";
    case cypher_filesystem_error_code_t::ERR_UNSUPPORTED_BACKEND:
        return "ERR_UNSUPPORTED_BACKEND";
    case cypher_filesystem_error_code_t::ERR_PERMISSION_DENIED:
        return "ERR_PERMISSION_DENIED";
    case cypher_filesystem_error_code_t::ERR_IO_ERROR:
        return "ERR_IO_ERROR";
    default:
        return "ERR_UNKNOWN";
    }
}

constexpr inline const char *CypherFileSystem_ErrorDesc( const cypher_filesystem_error_code_t error ) {
    switch ( error ) {
    case cypher_filesystem_error_code_t::OK:
        return "operation completed successfully";
    case cypher_filesystem_error_code_t::ERR_NOT_INIT:
        return "fs subsystem is not initialized";
    case cypher_filesystem_error_code_t::ERR_IS_INIT:
        return "fs subsystem is already initialized";
    case cypher_filesystem_error_code_t::ERR_INVALID_PATH:
        return "invalid filesystem path";
    case cypher_filesystem_error_code_t::ERR_INVALID_MODE:
        return "invalid filesystem open mode";
    case cypher_filesystem_error_code_t::ERR_INVALID_HANDLE:
        return "invalid filesystem file handle";
    case cypher_filesystem_error_code_t::ERR_INVALID_ARGUMENT:
        return "invalid filesystem argument";
    case cypher_filesystem_error_code_t::ERR_TOO_MANY_MOUNTS:
        return "filesystem mount table is full";
    case cypher_filesystem_error_code_t::ERR_MOUNT_NOT_FOUND:
        return "filesystem mount was not found";
    case cypher_filesystem_error_code_t::ERR_PATH_NOT_FOUND:
        return "filesystem path was not found";
    case cypher_filesystem_error_code_t::ERR_FILE_OPEN_FAILED:
        return "file open failed";
    case cypher_filesystem_error_code_t::ERR_FILE_CLOSE_FAILED:
        return "file close failed";
    case cypher_filesystem_error_code_t::ERR_FILE_READ_FAILED:
        return "file read failed";
    case cypher_filesystem_error_code_t::ERR_FILE_WRITE_FAILED:
        return "file write failed";
    case cypher_filesystem_error_code_t::ERR_FILE_SEEK_FAILED:
        return "file seek failed";
    case cypher_filesystem_error_code_t::ERR_FILE_TELL_FAILED:
        return "file tell failed";
    case cypher_filesystem_error_code_t::ERR_BUFFER_TOO_SMALL:
        return "provided filesystem buffer is too small";
    case cypher_filesystem_error_code_t::ERR_UNSUPPORTED_BACKEND:
        return "filesystem backend is unsupported for this operation";
    case cypher_filesystem_error_code_t::ERR_PERMISSION_DENIED:
        return "filesystem permission denied";
    case cypher_filesystem_error_code_t::ERR_IO_ERROR:
        return "filesystem IO error";
    default:
        return "unknown filesystem error";
    }
}

constexpr inline common::cypher_common_error_t CypherFileSystem_ErrorCode( cypher_filesystem_error_code_t  error ) {
    return common::CypherCommon_ErrorMake( common::cypher_common_domain_t::COM_DOMAIN_FS, static_cast<common::com_u16>( error ) );
}

}

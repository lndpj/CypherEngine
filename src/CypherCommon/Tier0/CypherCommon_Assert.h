#ifndef CYPHER_COMMON_TIER0_ASSERT_H
#define CYPHER_COMMON_TIER0_ASSERT_H
#pragma once

/*
================
CypherCommon Assert

Debug assertions and always-evaluated verification macros.

Rules:
- No logging dependency.
- No filesystem dependency.
- Optional handler hook for higher layers.
================
*/

#include "CypherCommon_Debug.h"
#include "CypherCommon_Defines.h"

namespace cypher::common
{

using assert_handler_t = void ( * )( const char *expression,
                                     const char *message,
                                     const char *file,
                                     int line,
                                     const char *function_name );

// Returns the process-local assert callback slot.
inline assert_handler_t &AssertHandler()
{
    static assert_handler_t handler = nullptr;
    return handler;
}

// Installs a higher-level assert callback without depending on logging.
inline void SetAssertHandler( assert_handler_t handler )
{
    AssertHandler() = handler;
}

// Dispatches an assert failure to the optional callback.
inline void HandleAssert( const char *expression,
                          const char *message,
                          const char *file,
                          int line,
                          const char *function_name )
{
    assert_handler_t handler = AssertHandler();
    if ( handler != nullptr ) {
        handler( expression, message, file, line, function_name );
    }
}

} // namespace cypher::common

/*
================
Assert Macros
================
*/
#define CYPHER_STATIC_ASSERT( expression, message ) static_assert( expression, message )

#if CYPHER_BUILD_DEBUG
    #define CYPHER_ASSERT( expression )                                                         \
        do {                                                                                     \
            if ( CYPHER_UNLIKELY( !( expression ) ) ) {                                          \
                ::cypher::common::HandleAssert( #expression, nullptr, CYPHER_FILE, CYPHER_LINE,  \
                                                CYPHER_FUNCTION_NAME );                          \
                CYPHER_DEBUG_BREAK();                                                           \
            }                                                                                    \
        } while ( 0 )

    #define CYPHER_ASSERT_MSG( expression, message )                                             \
        do {                                                                                     \
            if ( CYPHER_UNLIKELY( !( expression ) ) ) {                                          \
                ::cypher::common::HandleAssert( #expression, message, CYPHER_FILE, CYPHER_LINE,  \
                                                CYPHER_FUNCTION_NAME );                          \
                CYPHER_DEBUG_BREAK();                                                           \
            }                                                                                    \
        } while ( 0 )
#else
    #define CYPHER_ASSERT( expression )                  do { } while ( 0 )
    #define CYPHER_ASSERT_MSG( expression, message )     do { } while ( 0 )
#endif

#define CYPHER_VERIFY( expression )                                                             \
    do {                                                                                         \
        if ( CYPHER_UNLIKELY( !( expression ) ) ) {                                              \
            ::cypher::common::HandleAssert( #expression, nullptr, CYPHER_FILE, CYPHER_LINE,      \
                                            CYPHER_FUNCTION_NAME );                              \
            CYPHER_DEBUG_ONLY( CYPHER_DEBUG_BREAK() );                                           \
        }                                                                                        \
    } while ( 0 )

#define CYPHER_VERIFY_MSG( expression, message )                                                 \
    do {                                                                                         \
        if ( CYPHER_UNLIKELY( !( expression ) ) ) {                                              \
            ::cypher::common::HandleAssert( #expression, message, CYPHER_FILE, CYPHER_LINE,      \
                                            CYPHER_FUNCTION_NAME );                              \
            CYPHER_DEBUG_ONLY( CYPHER_DEBUG_BREAK() );                                           \
        }                                                                                        \
    } while ( 0 )

#endif // CYPHER_COMMON_TIER0_ASSERT_H

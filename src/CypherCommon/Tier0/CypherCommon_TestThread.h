#ifndef CYPHER_COMMON_TIER0_TESTTHREAD_H
#define CYPHER_COMMON_TIER0_TESTTHREAD_H
#pragma once

/*
================
CypherCommon Test Thread

Small thread test harness declarations.
================
*/

#include "CypherCommon_BaseTypes.h"

namespace cypher::common
{

using test_thread_proc_t = i32 ( * )( void *pUserData );

struct test_thread_result_t {
    i32 exit_code;
    bool_t completed;
};

test_thread_result_t TestThread_Run( test_thread_proc_t proc, void *pUserData );

} // namespace cypher::common

#endif // CYPHER_COMMON_TIER0_TESTTHREAD_H

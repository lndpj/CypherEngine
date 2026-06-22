#ifndef CYPHER_COMMON_TIER0_PROGRESSBAR_H
#define CYPHER_COMMON_TIER0_PROGRESSBAR_H
#pragma once

/*
================
CypherCommon Progress Bar

Progress reporting declarations for command-line tools.
================
*/

#include "CypherCommon_BaseTypes.h"

namespace cypher::common
{

struct progress_bar_t {
    const char *pTitle;
    u64 total_work;
    u64 completed_work;
};

void ProgressBar_Begin( progress_bar_t *pProgress, const char *pTitle, u64 total_work );
void ProgressBar_Update( progress_bar_t *pProgress, u64 completed_work );
void ProgressBar_End( progress_bar_t *pProgress );

} // namespace cypher::common

#endif // CYPHER_COMMON_TIER0_PROGRESSBAR_H

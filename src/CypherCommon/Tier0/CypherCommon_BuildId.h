#ifndef CYPHER_COMMON_TIER0_BUILDID_H
#define CYPHER_COMMON_TIER0_BUILDID_H
#pragma once

/*
================
CypherCommon Build ID

Build identity declarations used by logs, crash reports, tools, diagnostics
and editor about dialogs.
================
*/

#include "CypherCommon_BaseTypes.h"

namespace cypher::common
{

struct build_id_t {
    const char *pProductName;
    const char *pInternalName;
    const char *pBranchName;
    const char *pCommitHash;
    const char *pBuildDate;
    const char *pBuildTime;
    version_t version;
};

const build_id_t *Cy_BuildId_GetEngine();
const build_id_t *Cy_BuildId_GetGame();
void Cy_BuildId_Format( const build_id_t &buildId, char *pDest, usize cchDest );

} // namespace cypher::common

#endif // CYPHER_COMMON_TIER0_BUILDID_H

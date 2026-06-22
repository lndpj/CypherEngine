#ifndef CYPHER_COMMON_TIER0_ANNOTATIONS_H
#define CYPHER_COMMON_TIER0_ANNOTATIONS_H
#pragma once

/*
================
CypherCommon Annotations

Portable annotation macro surface for future static analysis.
================
*/

#ifndef CY_IN
    #define CY_IN
#endif

#ifndef CY_OUT
    #define CY_OUT
#endif

#ifndef CY_INOUT
    #define CY_INOUT
#endif

#ifndef CY_OPTIONAL
    #define CY_OPTIONAL
#endif

#ifndef CY_CAP
    #define CY_CAP( count )
#endif

#ifndef CY_Z
    #define CY_Z
#endif

#endif // CYPHER_COMMON_TIER0_ANNOTATIONS_H

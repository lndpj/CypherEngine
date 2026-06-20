#ifndef CYPHER_ENGINE_PAK_H
#define CYPHER_ENGINE_PAK_H

#pragma once

#include "CypherPak_Compression.h"
#include "CypherPak_Error.h"
#include "CypherPak_Format.h"
#include "CypherPak_Reader.h"
#include "CypherPak_Types.h"
#include "CypherPak_Writer.h"

namespace cypher::engine::pak
{

/*
================
CypherPak

Runtime package archive backend used by the filesystem for .cypak mounts.
Tools will use the writer API later to build deterministic content packages.
================
*/

}       // namespace cypher::engine::pak

#endif // CYPHER_ENGINE_PAK_H

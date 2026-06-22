#include "CypherCommon_String.h"

#include <benchmark/benchmark.h>

using namespace cypher::common;

namespace
{

constexpr const char *kShortLower = "cypher";
constexpr const char *kShortUpper = "CYPHER";
constexpr const char *kMediumA = "textures/world/industrial/wall_panel_01_albedo.dds";
constexpr const char *kMediumB = "textures/world/industrial/wall_panel_01_normal.dds";
constexpr const char *kMediumCase = "TEXTURES/WORLD/INDUSTRIAL/WALL_PANEL_01_ALBEDO.DDS";
constexpr const char *kLongA =
    "scripts/entities/weapons/projectiles/arena_survival/rocket_launcher/projectile_rocket_fast.cfg";
constexpr const char *kLongB =
    "scripts/entities/weapons/projectiles/arena_survival/rocket_launcher/projectile_rocket_slow.cfg";

void BM_Cy_strlen_Short( benchmark::State &state )
{
    for ( auto _ : state ) {
        benchmark::DoNotOptimize( Cy_strlen( kShortLower ) );
    }
}

void BM_Cy_strlen_Medium( benchmark::State &state )
{
    for ( auto _ : state ) {
        benchmark::DoNotOptimize( Cy_strlen( kMediumA ) );
    }
}

void BM_Cy_strlen_Long( benchmark::State &state )
{
    for ( auto _ : state ) {
        benchmark::DoNotOptimize( Cy_strlen( kLongA ) );
    }
}

void BM_Cy_strnlen_Capped( benchmark::State &state )
{
    for ( auto _ : state ) {
        benchmark::DoNotOptimize( Cy_strnlen( kLongA, 32u ) );
    }
}

void BM_Cy_strisempty_NotEmpty( benchmark::State &state )
{
    for ( auto _ : state ) {
        benchmark::DoNotOptimize( Cy_strisempty( kShortLower ) );
    }
}

void BM_Cy_strisblank_NotBlank( benchmark::State &state )
{
    for ( auto _ : state ) {
        benchmark::DoNotOptimize( Cy_strisblank( kMediumA ) );
    }
}

void BM_Cy_strcmp_EqualShort( benchmark::State &state )
{
    for ( auto _ : state ) {
        benchmark::DoNotOptimize( Cy_strcmp( kShortLower, kShortLower ) );
    }
}

void BM_Cy_strcmp_DifferentMedium( benchmark::State &state )
{
    for ( auto _ : state ) {
        benchmark::DoNotOptimize( Cy_strcmp( kMediumA, kMediumB ) );
    }
}

void BM_Cy_strncmp_DifferentMediumCapped( benchmark::State &state )
{
    for ( auto _ : state ) {
        benchmark::DoNotOptimize( Cy_strncmp( kMediumA, kMediumB, 32u ) );
    }
}

void BM_Cy_stricmp_EqualShort( benchmark::State &state )
{
    for ( auto _ : state ) {
        benchmark::DoNotOptimize( Cy_stricmp( kShortLower, kShortUpper ) );
    }
}

void BM_Cy_stricmp_EqualMediumCase( benchmark::State &state )
{
    for ( auto _ : state ) {
        benchmark::DoNotOptimize( Cy_stricmp( kMediumA, kMediumCase ) );
    }
}

void BM_Cy_strnicmp_EqualMediumCaseCapped( benchmark::State &state )
{
    for ( auto _ : state ) {
        benchmark::DoNotOptimize( Cy_strnicmp( kMediumA, kMediumCase, 32u ) );
    }
}

void BM_Cy_strequal_EqualShort( benchmark::State &state )
{
    for ( auto _ : state ) {
        benchmark::DoNotOptimize( Cy_strequal( kShortLower, kShortLower ) );
    }
}

void BM_Cy_striequal_EqualMediumCase( benchmark::State &state )
{
    for ( auto _ : state ) {
        benchmark::DoNotOptimize( Cy_striequal( kMediumA, kMediumCase ) );
    }
}

void BM_Cy_strniequal_EqualMediumCaseCapped( benchmark::State &state )
{
    for ( auto _ : state ) {
        benchmark::DoNotOptimize( Cy_strniequal( kMediumA, kMediumCase, 32u ) );
    }
}

} // namespace

BENCHMARK( BM_Cy_strlen_Short );
BENCHMARK( BM_Cy_strlen_Medium );
BENCHMARK( BM_Cy_strlen_Long );
BENCHMARK( BM_Cy_strnlen_Capped );
BENCHMARK( BM_Cy_strisempty_NotEmpty );
BENCHMARK( BM_Cy_strisblank_NotBlank );
BENCHMARK( BM_Cy_strcmp_EqualShort );
BENCHMARK( BM_Cy_strcmp_DifferentMedium );
BENCHMARK( BM_Cy_strncmp_DifferentMediumCapped );
BENCHMARK( BM_Cy_stricmp_EqualShort );
BENCHMARK( BM_Cy_stricmp_EqualMediumCase );
BENCHMARK( BM_Cy_strnicmp_EqualMediumCaseCapped );
BENCHMARK( BM_Cy_strequal_EqualShort );
BENCHMARK( BM_Cy_striequal_EqualMediumCase );
BENCHMARK( BM_Cy_strniequal_EqualMediumCaseCapped );

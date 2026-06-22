# CypherEngine

CypherEngine is a learning-first C++20 game engine project built to understand how a real engine runtime is designed from the ground up.

The codebase favors C-style C++: plain structs, explicit ownership, free functions, module prefixes, data-oriented runtime code, and small subsystem boundaries instead of deep inheritance trees.

The project studies ideas from idTech, GoldSrc/Source, early CryEngine-era architecture, and similar professional engines. Those engines are references for architecture, naming discipline, tooling expectations, and performance mindset; CypherEngine is not a fork and does not copy their implementations.

## What This Is

CypherEngine is being built as a full runtime and toolchain foundation for a 3D game:

- runtime host loop and platform ownership
- C-style common library tiers for base types, platform macros, asserts, memory operations, string helpers, containers, commands, and engine utilities
- custom memory systems including arenas, pools, buckets, scratch allocation, and diagnostics
- virtual filesystem with path normalization, mounts, directory operations, file watching, async I/O direction, and package integration
- CypherPak archive format for deterministic engine-owned asset packages
- SDL3 windowing with OpenGL/glad rendering bootstrap
- renderer foundations for shaders, meshes, cameras, and draw submission
- math foundations for vectors, matrices, quaternions, bounds, rays, planes, and frustums
- config, command, cvar, logging, and error-code foundations
- future Mason editor for world authoring, asset workflows, live preview, and engine-integrated tools

## Dependencies

The project uses CMake as the build source of truth and vcpkg for third-party dependency management.

Current and planned dependency set:

- SDL3 for windowing and platform-facing application support
- OpenGL with vendored glad for graphics API loading
- Catch2 for unit tests
- Google Benchmark for performance baselines
- FreeType and HarfBuzz for font and text shaping work
- libpng and libjpeg-turbo for image loading support
- OpenAL Soft for audio
- libsodium for future cryptography/security utilities
- LZ4 and Zstd for compression
- xxHash for fast hashing
- meshoptimizer for mesh processing and optimization
- Tracy for profiling

Most engine systems are intended to be written in-house for learning and control. Third-party libraries are used where they provide a proven platform layer, tooling layer, codec, profiler, or security primitive that would be wasteful or risky to replace immediately.

## Build

```bash
cmake -S . -B build
cmake --build build
./build/bin/CypherEngine
```

With vcpkg:

```bash
cmake -S . -B build \
  -DCMAKE_TOOLCHAIN_FILE="$PWD/vcpkg/scripts/buildsystems/vcpkg.cmake"
cmake --build build
```

## Tests

```bash
cmake -S . -B build \
  -DCYPHERENGINE_BUILD_TESTS=ON \
  -DCMAKE_TOOLCHAIN_FILE="$PWD/vcpkg/scripts/buildsystems/vcpkg.cmake"
cmake --build build
ctest --test-dir build --output-on-failure --no-tests=error
```

Current tests cover the low-level common smoke checks, Tier1 string helpers, filesystem smoke coverage, and CypherPak smoke coverage.

## Benchmarks

```bash
cmake -S . -B build-bench \
  -DCMAKE_BUILD_TYPE=Release \
  -DCYPHERENGINE_BUILD_BENCHMARKS=ON \
  -DCMAKE_TOOLCHAIN_FILE="$PWD/vcpkg/scripts/buildsystems/vcpkg.cmake"
cmake --build build-bench --config Release
./build-bench/bin/cypher_common_string_bench
./build-bench/bin/cypher_memory_bench
./build-bench/bin/cypher_filesystem_path_bench
./build-bench/bin/cypher_pak_bench
```

Benchmarks are separate from the normal build. They are used to inspect real costs while developing strings, memory allocators, VFS path code, package archives, and other performance-sensitive systems.

## Documentation

Start with:

- [docs/index.md](docs/index.md)
- [docs/current_status.md](docs/current_status.md)
- [docs/roadmap.md](docs/roadmap.md)
- [docs/architecture.md](docs/architecture.md)
- [docs/subsystems.md](docs/subsystems.md)
- [docs/coding_style.md](docs/coding_style.md)

## License

The repository currently contains a GPL-2.0 license file.

If CypherEngine is moved to a proprietary license later, the license file, README, dependency policy, and contributor policy must be changed together. A proprietary engine license is possible only if the project owner has the rights to all code being relicensed and the third-party dependencies allow the intended distribution model.

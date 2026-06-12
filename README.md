# CypherEngine

CypherEngine is a learning-first C++20 game engine project. The goal is to build a real runtime step by step while understanding the engine architecture, tradeoffs, and data flow behind each subsystem.

The engine is intentionally explicit and C-style: simple structs, free functions, clear ownership, and data-oriented runtime code are preferred over deep object hierarchies or clever abstractions.

## Current State

The repository is still early, but the runtime foundation is now present. Current work includes:

- core types, logging, errors, platform helpers, and host loop ownership
- SDL3 windowing and OpenGL bootstrap
- renderer basics for shaders, meshes, cameras, and draw submission
- math helpers for vectors, matrices, quaternions, bounds, rays, planes, and frustums
- command, cvar, and config loading foundations
- VFS path mounting and virtual-to-physical file access
- CypherPak package format work for engine-owned asset packaging
- early memory, resource, world, and toolchain planning

CypherEngine uses Quake, idTech, GoldSrc/Source, and early CryEngine as references for ideas and architecture, not as source to copy wholesale.

## Near-Term Path

The next path is deliberately small:

1. finish VFS and CypherPak
2. finish memory arenas and basic pools
3. build the resource layer for shaders, meshes, textures, and materials
4. strengthen renderer ownership and draw submission
5. add world loading and gameplay-facing runtime pieces
6. build toward a playable loop
7. add CypherStudio later, after the runtime is solid

## Documentation

Start with:

- [docs/index.md](docs/index.md)
- [docs/current_status.md](docs/current_status.md)
- [docs/roadmap.md](docs/roadmap.md)
- [docs/architecture.md](docs/architecture.md)
- [docs/subsystems.md](docs/subsystems.md)
- [docs/coding_style.md](docs/coding_style.md)

## Build

```bash
cmake -S . -B build
cmake --build build
./build/bin/cypherengine
```

CMake is the build source of truth.

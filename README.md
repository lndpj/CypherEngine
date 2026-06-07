# CypherEngine

CypherEngine is a from-scratch C++20 3D engine runtime built by Cypher Software for arena survival FPS experiments, custom tools, and future games.

The project is inspired by the engineering spirit of early idTech, Quake, GoldSrc/Source, and early CryEngine: explicit systems, readable C/C++ style APIs, engine-owned formats, and a runtime that can eventually grow into an editor-driven toolchain.

## Direction

The current technical direction is:

- `SDL3` for the platform/window layer
- `OpenGL` through `GLAD` for the first renderer backend
- engine-owned filesystem, math, shader, mesh, camera, and draw submission layers
- engine-owned memory arenas, pools, resource handles, and asset lifetime tracking
- custom world, map, model, material, package, and resource formats when the runtime pressure is real
- a later all-in-one Qt editor direction, similar in spirit to CryEngine Sandbox and Source/Hammer workflows
- long-term client/server, networking, entity/component, audio, physics, AI, animation, asset pipeline, and scripting support

The reference engines are used as learning material and architectural inspiration, not as code to copy blindly.

## Current State

The codebase is still early, but the runtime is now real enough to open a window, create an OpenGL context, load shaders through the engine filesystem path, submit a mesh, and draw through the renderer path.

Today the repository contains early engine source under `src/CypherEngine/` with these runtime foundations in place:

- `CypherCommon`
  - core types
  - packed cross-subsystem error surface
  - formatted print/error helpers
- `CypherLog`
  - runtime config
  - level filtering
  - channel masking
  - console/file sink path
- `CypherSystem`
  - platform/compiler detection
  - path helpers
  - monotonic time and sleep helpers
  - SDL3 window creation and event polling
- `CypherMemory`
  - arena allocator design in progress
  - allocation errors, flags, markers, counters, and future backing strategy
- `CypherFileSystem`
  - mount table
  - virtual-to-physical path resolution
  - file open/read/write/seek/tell
  - read-entire-file helper
- `CypherHost`
  - top-level runtime stage and frame loop ownership
- `CypherRender`
  - renderer lifecycle
  - OpenGL context/bootstrap
  - GLAD function loading
  - frame begin/end
  - shader loading/compilation/linking
  - mesh upload/draw path
  - camera matrices
  - draw item/list submission
- `CypherMath`
  - vector, matrix, quaternion, bounds, ray, plane, and frustum helpers
- `CypherCommand`
  - command registry
  - argument parsing
  - callback dispatch
- `CypherCVar`
  - cvar registry
  - typed cached values
  - flags and mutation path
- `CypherConfig`
  - cfg file loading
  - single-line execution
  - `exec`, `set`, `seta`, and command fallback

The repository now also carries the future subsystem skeleton, inspired by the way early CryEngine separated runtime, renderer, game, editor, and tool code:

- `CypherPlatform`
- `CypherInput`
- `CypherResource`
- `CypherWorld`
- `CypherEntity`
- `CypherPhysics`
- `CypherAudio`
- `CypherAI`
- `CypherAnimation`
- `CypherNetwork`
- `CypherScript`
- `CypherProfile`
- `CypherConsole`
- `CypherEditor`
- `CypherTools`

What does not exist yet:

- complete memory arena/pool backend
- input system and camera controller
- material and texture runtime
- real world/map runtime
- custom Cypher world/map/resource compiler path
- client/server split
- audio, physics, entity/component, AI, animation, networking, and gameplay runtime
- tools pipeline

## Near-Term Path

The immediate build order is:

1. finish the memory arena foundation and then add pool allocators
2. strengthen the filesystem around mounted roots, virtual paths, and arena-backed file reads
3. add the resource system for shaders, meshes, textures, materials, and future hot reload
4. add input system and fly-camera movement
5. clean renderer draw submission and frame ownership
6. add texture/material runtime
7. load simple engine-owned mesh/model data
8. move toward world loading, visibility, and editor/tool foundations

## Documentation Map

Start here:

- [docs/index.md](docs/index.md)
- [docs/current_status.md](docs/current_status.md)
- [docs/development_phases.md](docs/development_phases.md)
- [docs/roadmap.md](docs/roadmap.md)
- [docs/architecture.md](docs/architecture.md)
- [docs/subsystems.md](docs/subsystems.md)
- [docs/toolchain_plan.md](docs/toolchain_plan.md)
- [docs/project_structure.md](docs/project_structure.md)
- [docs/coding_style.md](docs/coding_style.md)
- [docs/CYPHERENGINE_API_REFERENCE.md](docs/CYPHERENGINE_API_REFERENCE.md)
- [docs/CYPHERENGINE_API_IMPLEMENTATION.md](docs/CYPHERENGINE_API_IMPLEMENTATION.md)

Long-lived project memory:

- [CHANGELOG.md](CHANGELOG.md)
- [docs/devlog/2026-04.md](docs/devlog/2026-04.md)
- [docs/adr/0001-coop-first-listen-server-architecture.md](docs/adr/0001-coop-first-listen-server-architecture.md)

## Build

Current build path:

```bash
cmake -S . -B build
cmake --build build
./build/bin/cypherengine
```

Later, a thin convenience wrapper can sit on top of this, but CMake remains the real build source of truth.

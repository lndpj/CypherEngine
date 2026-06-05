# CypherEngine

CypherEngine is a from-scratch C++20 3D engine runtime built by Spark Software for arena survival FPS experiments, custom tools, and future games.

The project is inspired by the engineering spirit of early idTech, Quake, GoldSrc/Source, and early CryEngine: explicit systems, readable C/C++ style APIs, engine-owned formats, and a runtime that can eventually grow into an editor-driven toolchain.

## Direction

The current technical direction is:

- `SDL3` for the platform/window/input layer
- `OpenGL` through `GLAD` for the first renderer backend
- engine-owned filesystem, math, shader, mesh, camera, and draw submission layers
- custom map/model/material formats when the runtime pressure is real
- a later all-in-one editor direction, similar in spirit to old Sandbox-style workflows
- long-term client/server, networking, ECS, audio, physics, asset pipeline, and scripting support

The reference engines are used as learning material and architectural inspiration, not as code to copy blindly.

## Current State

The codebase is still early, but the runtime is now real enough to open a window, create an OpenGL context, load shaders through the engine filesystem path, submit a mesh, and draw through the renderer path.

Today the repository contains roughly `9.1k` lines of engine source under `src/CypherEngine/` with these runtime foundations in place:

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

What does not exist yet:

- input system and camera controller
- material and texture runtime
- real world/map runtime
- BSP or custom `rbsp` loader/compiler path
- client/server split
- memory arenas/resource lifetime model
- audio, physics, ECS, networking, and gameplay runtime
- tools pipeline

## Near-Term Path

The immediate build order is:

1. clean renderer draw submission and frame ownership
2. add input system and fly-camera movement
3. add texture/material runtime
4. add resource lifetime and memory arena groundwork
5. load simple engine-owned mesh/model data
6. move toward map loading, visibility, and editor/tool foundations

## Documentation Map

Start here:

- [docs/index.md](/Users/karlosiric/Documents/MyProjects/REAP/docs/index.md)
- [docs/current_status.md](/Users/karlosiric/Documents/MyProjects/REAP/docs/current_status.md)
- [docs/development_phases.md](/Users/karlosiric/Documents/MyProjects/REAP/docs/development_phases.md)
- [docs/roadmap.md](/Users/karlosiric/Documents/MyProjects/REAP/docs/roadmap.md)
- [docs/architecture.md](/Users/karlosiric/Documents/MyProjects/REAP/docs/architecture.md)
- [docs/subsystems.md](/Users/karlosiric/Documents/MyProjects/REAP/docs/subsystems.md)
- [docs/toolchain_plan.md](/Users/karlosiric/Documents/MyProjects/REAP/docs/toolchain_plan.md)
- [docs/CYPHERENGINE_API_REFERENCE.md](/Users/karlosiric/Documents/MyProjects/REAP/docs/CYPHERENGINE_API_REFERENCE.md)
- [docs/CYPHERENGINE_API_IMPLEMENTATION.md](/Users/karlosiric/Documents/MyProjects/REAP/docs/CYPHERENGINE_API_IMPLEMENTATION.md)

Long-lived project memory:

- [CHANGELOG.md](/Users/karlosiric/Documents/MyProjects/REAP/CHANGELOG.md)
- [docs/devlog/2026-04.md](/Users/karlosiric/Documents/MyProjects/REAP/docs/devlog/2026-04.md)
- [docs/adr/0001-coop-first-listen-server-architecture.md](/Users/karlosiric/Documents/MyProjects/REAP/docs/adr/0001-coop-first-listen-server-architecture.md)

## Build

Current build path:

```bash
cmake -S . -B build
cmake --build build
./build/bin/cypherengine
```

Later, a thin convenience wrapper can sit on top of this, but CMake remains the real build source of truth.

# CypherEngine Current Status

## Project state

CypherEngine is still in early foundation stage, but the runtime stack is now more real than the old docs implied.

`CypherEngine` is the native engine runtime currently living in this repository.

Current code snapshot:

- early runtime modules live under `src/CypherEngine/`
- build succeeds through CMake
- the executable target is `cypherengine`
- the runtime can create a window, initialize OpenGL, load shaders, and submit a basic mesh path
- a CryEngine-inspired future subsystem skeleton now exists for editor, tools, resources, world, input, physics, audio, AI, animation, networking, scripting, and profiling

## What exists now

- foundational types and shared error surface
- common formatted print/error helpers
- memory arena design and implementation work in progress
- logging runtime
- early platform runtime helpers
- host lifecycle scaffold
- SDL3 window creation and event polling
- filesystem mount/read path
- OpenGL context bootstrap through GLAD
- renderer lifecycle, shader, mesh, camera, and draw-list path
- vector, matrix, quaternion, bounds, ray, plane, and frustum math
- command system backend
- cvar system backend
- cfg loading/execution backend
- documentation/process system

## What is done-for-now

- `CypherCommon`
  - type aliases
  - sentinel values
  - packed subsystem error representation
  - formatted print/error helpers
- `CypherMemory`
  - arena allocator API in progress
  - size helpers
  - allocation flags
  - markers and rewind model
  - allocation counters and stats direction
- `CypherLog`
  - runtime state
  - level/channel filtering
  - console/file output path
- `CypherSystem`
  - platform/compiler detection
  - path construction
  - monotonic time/sleep/local-time helper
  - SDL3 window creation and event polling
- `CypherHost`
  - startup/shutdown ownership
  - frame begin/update/render/end sequencing
- `CypherFileSystem`
  - mount table
  - loose-file read/write path
  - read-entire-file helper for shaders/assets
- `CypherRender`
  - init/shutdown state
  - in-frame state validation
  - error-coded lifecycle contract
  - GL context bootstrap
  - shader registry/loading
  - mesh upload/draw path
  - camera matrices
  - draw-list submission
- `CypherMath`
  - vector/matrix/quaternion foundations
  - bounds/ray/plane/frustum geometry helpers
- `CypherCommand`
  - command registry
  - duplicate prevention
  - fixed argument parsing
  - callback execution
- `CypherCVar`
  - fixed registry
  - typed cached values
  - flags
  - set/find/get path
- `CypherConfig`
  - file loading
  - line execution
  - `exec`
  - `set`
  - `seta`
  - fallback command dispatch

## Active milestone

`M5 - Memory, Resources, and Runtime Ownership Foundation`

This milestone is complete only when:

- `CypherMemory` has working arenas, markers, rewinds, counters, and shutdown behavior
- pool allocator design exists for long-lived entities/resources that cannot use pure linear reset
- `CypherFileSystem` can read assets into caller-provided/arena-backed memory cleanly
- `CypherResource` has a first asset-handle direction for shaders, meshes, textures, and materials
- renderer draw submission keeps clear ownership over what is temporary, what is persistent, and who frees it

## Immediate next tasks

1. finish the `CypherMemory` arena allocator
2. add pool allocator design on top of the arena model
3. strengthen `CypherFileSystem` around mounted roots and arena-backed file reads
4. introduce `CypherResource` as the asset lifetime layer
5. return to input, fly-camera, and renderer material/texture work
6. keep docs and API references aligned as CypherEngine grows

## Explicitly not active yet

- real custom world/map runtime implementation
- client/server networking
- gameplay loop
- custom model/material/archive tooling
- custom editor tooling
- VM/game-script runtime

These are all intended, but they are not the current coding target.

## Resume rule

If work pauses, resume from this file first.

Do not restart architecture design from scratch unless this file and the surrounding docs are intentionally updated with a new direction.

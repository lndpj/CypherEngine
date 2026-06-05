# CypherEngine Current Status

## Project state

CypherEngine is still in early foundation stage, but the runtime stack is now more real than the old docs implied.

`CypherEngine` is the native engine runtime currently living in this repository.

Current code snapshot:

- roughly `9.1k` lines of engine source under `src/CypherEngine/`
- early runtime modules live under `src/CypherEngine/`
- build succeeds through CMake
- the executable target is `cypherengine`
- the runtime can create a window, initialize OpenGL, load shaders, and submit a basic mesh path

## What exists now

- foundational types and shared error surface
- common formatted print/error helpers
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
  - numeric constants
  - sentinel values
  - packed subsystem error representation
  - formatted print/error helpers
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

`M4 - Local 3D Runtime Foundation`

This milestone is complete only when:

- the draw-list path is clean enough to survive real objects
- camera movement is driven by a real input subsystem
- shader uniforms and renderer state are stable enough for textured meshes
- resource lifetime has a clear memory ownership plan
- the test triangle path is replaced by reusable renderer-facing objects

## Immediate next tasks

1. clean renderer draw submission and remove remaining temporary test assumptions
2. add the input subsystem and fly-camera controller
3. add material/texture runtime scaffolding
4. introduce memory arena/resource lifetime design
5. keep docs and API references aligned as CypherEngine grows

## Explicitly not active yet

- real BSP/custom map runtime implementation
- client/server networking
- gameplay loop
- custom model/material/archive tooling
- custom editor tooling
- VM/game-script runtime

These are all intended, but they are not the current coding target.

## Resume rule

If work pauses, resume from this file first.

Do not restart architecture design from scratch unless this file and the surrounding docs are intentionally updated with a new direction.

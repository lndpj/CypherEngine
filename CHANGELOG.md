# CypherEngine Changelog

All notable changes to REAP and the CypherEngine runtime are tracked here.

## [0.1.0] - 2026-06-05

### Changed
- Kept public subsystem functions on the branded `CypherRender_*`, `CypherSystem_*`, `CypherCommon_*` style.
- Cleaned subsystem types back to namespace-local names such as `render::shader_t`, `render::mesh_t`, `sys::window_t`, `host::state_t`, and `common::error_t`.
- Updated CypherEngine build metadata to describe the custom idTech/GoldSrc/early CryEngine-inspired runtime direction.

### Verified
- Confirmed the cleaned naming pass builds successfully with CMake using a fresh verification build directory.

## [0.1.0] - 2026-06-02

### Added
- Added the first frustum math implementation for renderer visibility work:
  - Gribb-Hartmann projection-view plane extraction
  - frustum point containment
  - frustum-vs-bounds intersection testing
- Added plane construction helpers for building planes from point/normal pairs and triangle points.

### Fixed
- Fixed frustum bounds testing to use real AABB half-extents when projecting bounds onto frustum plane normals.

## [0.1.0] - 2026-05-31

### Added
- Designed the camera frustum path around six world-space planes extracted from `projection * view`.
- Documented the renderer culling direction: object/world bounds are tested against camera frustum planes before draw submission.

## [0.1.0] - 2026-05-26

### Added
- Added `math_bounds` for AABB creation, expansion, center/size queries, point containment, and bounds overlap.
- Added `math_ray` for ray point evaluation, ray-plane intersection, and ray-bounds slab intersection.
- Added `ray_t` to the math type layer for future traces, picking, collision, and weapon-fire tests.

## [0.1.0] - 2026-05-25

### Added
- Added `math_plane` for signed plane distance and front/back/on-plane point classification.
- Added `plane_t`, `bounds_t`, and frustum-oriented math type definitions as the geometry foundation for BSP, collision, and renderer culling.

## [0.1.0] - 2026-05-24

### Added
- Added the first custom engine math library pass:
  - vector helpers
  - matrix helpers
  - quaternion helpers
  - projection/view/model transform support
- Added quaternion rotation support including construction, normalization, inverse/conjugate, multiplication, vector rotation, matrix conversion, NLerp, and Slerp.

### Changed
- Chose an OpenGL-style math convention for the renderer path:
  - column-major storage
  - column-vector transforms
  - right-handed world convention
  - `projection * view * model` transform order

## [0.1.0] - 2026-05-06

### Added
- Added SDL3 window creation and runtime event plumbing through the `sys` and `host` layers.
- Added the first OpenGL renderer backend path with SDL GL context creation, GLAD function loading, frame begin/end, and buffer clearing.
- Added shader, mesh, and renderer scaffolding for moving from test drawing toward real engine assets.

### Changed
- Moved host startup toward a cleaner orchestration path where `main` stays minimal and host owns subsystem initialization.

## [0.1.0] - 2026-05-02

### Added
- Added cleanup of the host initializaion

## [0.1.0] - 2026-04-27

### Added
- Added game/engine identity direction for `REAP`, `CypherEngine`, internal `CypherEngine`, and `Spark Software`.
- Started shaping the proper `sys_` platform layer around startup descriptors, platform/compiler identity, paths, time, sleep, and local-time services.

### Changed
- Removed bulky comments from public engine headers so API surfaces are easier to read while the engine architecture is still forming.
- Cleaned `sys_platform.h` into a compact platform API surface.
- Hid VFS runtime state inside `fs_main.cpp` instead of exposing it through public headers.

### Fixed
- Fixed cfg comment stripping so `//` comments stop parsing at the correct point instead of breaking the scan too early.
- Kept the project building cleanly after the header cleanup and VFS implementation work.

## [0.1.0] - 2026-04-26

### Added
- Added the first `fs` virtual filesystem subsystem:
  - mount table
  - virtual-to-physical path resolution
  - write path storage
  - file open/close
  - read/write
  - seek/tell
  - read-entire-file helper
- Added VFS error/type/API headers for the initial OS-file backend.
- Added docs describing the project direction:
  - `REAP` as the game/project
  - `CypherEngine` as the native engine runtime
  - `SDL3`, `OpenGL`, `Quake III BSP`, `rmdl`, and `rpk` as the long-term technical path.
- Added API documentation anchors:
  - `docs/CYPHERENGINE_API_REFERENCE.md`
  - `docs/CYPHERENGINE_API_IMPLEMENTATION.md`

### Changed
- Completed the cfg system enough to load files and dispatch parsed lines through `cfg_execute_line`.
- Updated docs and roadmap away from the earlier Raylib direction and toward the current custom runtime path.

## [0.1.0] - 2026-04-25

### Added
- Continued cfg system implementation.
- Added cfg single-line execution support for:
  - `exec`
  - `set`
  - `seta`
  - command fallback through `cmd_execute`

### Changed
- Consolidated cfg parsing so file loading delegates per-line behavior through the same execution path.

## [0.1.0] - 2026-04-24

### Added
- Added cvar mutation support through `cvar_set`.
- Started the cfg subsystem for engine/game configuration loading.
- Added early cfg file parsing and command-line interpretation work.

### Changed
- Treated cfg as the next bridge between cvars, commands, and future filesystem-backed startup config.

## [0.1.0] - 2026-04-23

### Added
- Expanded the cvar subsystem with bool parsing and typed cached values.
- Finalized the first version of `cvar_register`.

### Changed
- Improved cvar default value handling and flag validation.

## [0.1.0] - 2026-04-22

### Added
- Added the command subsystem with:
  - fixed command registry
  - command registration
  - command lookup
  - argument parsing
  - callback execution
- Added the cvar subsystem API/error foundation.
- Started cvar implementation with registry, flags, and typed storage.

### Changed
- Cleaned the command path enough for cfg/cvar integration work to build on it.

## [0.1.0] - 2026-04-21

### Added
- Added subsystem-local error enums and packed common error conversion helpers.
- Added `com_printf`, `com_dprintf`, and `com_errorf` for common output and surfaced error printing.
- Added nicer domain/error formatted output for common error reporting.
- Started command-system design and early parsing work.

### Changed
- Improved logging and error printing flow so hard surfaced errors can show subsystem domains and packed hex codes.

## [0.1.0] - 2026-04-20

### Added
- Added early `sys_` platform helpers:
  - platform detection
  - compiler detection
  - basename helper
  - monotonic time helper
  - local time helper
- Added renderer lifecycle scaffolding and then simplified it back toward a cleaner runtime contract.
- Added `sys_` and `com_` naming alignment across early engine APIs.
- Added the first changelog/documentation pass.

### Changed
- Moved foundation code into the `common` / `com_` naming path.
- Continued aligning the project around a subsystem-first architecture.

## [0.1.0] - 2026-04-19

### Added
- Added early `CypherEngine` runtime fundamentals.
- Added host/app lifecycle scaffolding.
- Started the logging subsystem:
  - log types
  - log API declarations
  - first implementation path
- Added early top-level runtime conductor work.

## [0.1.0] - 2026-04-18

### Added
- Created the CypherEngine repository foundation.
- Added initial CMake-based project scaffold.
- Added `src` and `thirdparty` layout.
- Added initial engine foundation header and baseline docs/process files.

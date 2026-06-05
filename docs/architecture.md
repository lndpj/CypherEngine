# CypherEngine Architecture

CypherEngine is a layered project made of three connected bodies of work:

- `CypherEngine` engine runtime
- game logic
- tools and offline pipeline

The long-term structure follows Quake/idTech-style separation, with CryEngine-style subsystem naming adapted to modern C++.

## Top-level architecture

### `src/`

The native engine runtime.

Current and target modules:

- `CypherCommon`
- `CypherSystem`
- `CypherFileSystem`
- `CypherLog`
- `CypherCommand`
- `CypherCVar`
- `CypherConfig`
- `CypherHost`
- `CypherMath`
- `CypherRender`
- `CypherClient`
- `CypherServer`
- `CypherNetwork`
- `CypherPhysics`
- `CypherAudio`
- `CypherECS`
- `CypherScript`

### `rvm/`

Standalone Cypher VM project.

Owns:

- VM runtime
- assembler
- disassembler
- later the REAP Script compiler

### `game/`

Gameplay scripts intended to run on the VM.

This is where high-level gameplay should live once the VM path is real:

- players
- waves
- enemies
- combat
- items
- rules

### `tools/`

Offline pipeline executables.

Owns:

- model compiler/decompiler
- texture pipeline
- archive tooling
- small engine-owned editor tools when justified

## Boundary rules

- `CypherCommon` is shared foundation
- `CypherRender` owns GPU-facing code
- `CypherSystem` owns OS/window/time/platform-facing behavior and the SDL seam
- `CypherFileSystem` owns path resolution, mounts, and file I/O
- `CypherHost` owns top-level engine orchestration
- `CypherClient` owns local input/prediction/presentation bridge
- `CypherServer` owns authoritative simulation
- `CypherNetwork` owns transport and serialization primitives
- `CypherPhysics` owns movement and shared simulation code
- `CypherAudio` owns sound runtime
- `CypherECS` owns entity/component integration layer
- `CypherScript` is the bridge between engine runtime and `rvm`

## Current implementation reality

The current repository is much earlier than the target structure.

Today:

- code is concentrated in `src/CypherEngine/`
- the project has core runtime, SDL3 windowing, OpenGL bootstrap, math, shader, mesh, and camera foundations
- command/cvar/cfg/filesystem subsystems already exist as early engine services
- the next major missing seam is input, material/texture runtime, memory/resource ownership, and real world content

That is acceptable for now, as long as new work follows the documented target structure from this point forward.

## Design philosophy

- engine-owned interfaces at subsystem boundaries
- explicit data flow
- no heavy abstraction without runtime pressure
- learn from Quake/DOOM lineage without copying blindly
- long-term maintainability for a solo developer

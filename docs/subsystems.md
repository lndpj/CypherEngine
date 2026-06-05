# CypherEngine Subsystems

This file defines what the major CypherEngine modules are supposed to do.

## `CypherCommon`

Shared engine foundation used by both client and server style code.

Expected contents:

- engine bootstrap helpers
- memory helpers
- command system
- cvar system
- cfg/console backend
- filesystem
- pak/archive reading
- logging
- string helpers
- math
- common types

This is the layer everything else stands on.

## `CypherRender`

Owns:

- frame begin/end
- world rendering
- BSP rendering
- model rendering
- materials
- shaders
- particles
- lightmaps
- sky
- debug draw

Rule:

- the rest of the engine should not call `OpenGL` directly
- the renderer should use platform-created contexts, not leak platform APIs outward

## `CypherServer`

Owns:

- authoritative simulation
- client connection management
- snapshot generation
- receiving input
- sending world state
- game VM bridge on the authoritative side

## `CypherClient`

Owns:

- local input
- prediction
- interpolation
- view/camera
- client-side effects
- HUD
- scoreboard
- console UI
- menus

## `CypherNetwork`

Owns:

- socket-level communication
- packet framing
- reliable/unreliable channel logic
- serialization

Both client and server depend on this layer.

## `CypherMap`

Owns:

- BSP/custom map loading
- map format definitions
- traceline / tracebox
- PVS lookup
- entity lump parsing
- collision against brush geometry

## `CypherPhysics`

Owns:

- player movement
- slide movement
- step movement
- air movement
- shared movement code used by both prediction and authoritative simulation

## `CypherAudio`

Owns:

- audio runtime startup/shutdown
- loading sounds
- mixing
- spatial sound
- music playback

## `CypherECS`

Owns:

- entity/component definitions
- system registration
- prefab setup
- query helpers

This is the replacement for a monolithic entity array.

## `CypherScript`

Owns:

- loading the gameplay VM in the engine runtime
- trap bridge between VM and native engine code
- engine-side debugging of VM state

## `CypherSystem`

Owns:

- OS entry point details
- timing
- `SDL3` bootstrap and event pump seam
- file/path wrappers where needed
- OS socket differences where needed
- crash/error/platform-specific behavior

This is the engine/OS seam.

## `rvm`

Standalone virtual machine project.

Owns:

- VM runtime
- memory model
- stack
- opcodes
- loader
- debugger
- assembler
- disassembler
- later language compiler

## `game`

Owns gameplay logic intended to run on the VM:

- players
- weapons
- projectiles
- combat
- enemies
- AI
- waves
- spawn logic
- items
- triggers
- rules

## `tools`

Owns offline content pipeline:

- custom model compiler/decompiler
- texture processing
- package/archive creation and extraction
- small purpose-built editor tools
- asset build orchestration

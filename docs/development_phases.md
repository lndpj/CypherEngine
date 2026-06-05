# CypherEngine Development Phases

This is the detailed build order for REAP.

The rule is simple:

- each phase must leave the project in a more runnable or more teachable state
- do not build deep later-phase systems before earlier runtime pressure exists
- use the codebase you actually have, not the one you imagine you already finished

## Phase 0 - Foundation Core

Build:

- basic types
- common error surface
- print/error helpers
- logging
- platform scaffold
- host scaffold
- renderer contract

Exit when:

- the project builds
- the foundation API is documented

## Phase 1 - Command and Config Backbone

Build:

- command system backend
- cvar system backend
- cfg execution/loading
- basic startup-facing config path

Exit when:

- runtime commands can be registered and executed
- cvars can be registered, queried, and changed
- cfg files can drive startup/runtime configuration

Status:

- partially complete now

## Phase 2 - Filesystem and Asset Path

Build:

- virtual filesystem
- search paths / mounts
- loose-file loading through engine-owned APIs
- cfg migration onto filesystem APIs

Exit when:

- config and future assets no longer depend on raw ad-hoc file opens everywhere

## Phase 3 - SDL3 and OpenGL Runtime Bootstrap

Build:

- `SDL3` platform bootstrap
- window creation
- event pump
- `OpenGL` context creation
- renderer bootstrap against the real backend

Exit when:

- the engine can open a visible window and run a frame loop with the intended platform/render path

## Phase 4 - Local 3D Graybox

Build:

- camera
- input integration
- player movement
- one graybox arena
- debug visuals

Exit when:

- the player can move around a 3D space comfortably

## Phase 5 - Quake III BSP Runtime

Build:

- BSP loading
- entity lump parsing
- surface/lightmap/material runtime structures
- debug/runtime inspection support

Exit when:

- authored BSP content can replace pure graybox assumptions

## Phase 6 - Honest Runtime Split

Build:

- clearer client/server/runtime ownership
- low-level network groundwork
- session model

Exit when:

- multiplayer architecture is no longer only theoretical

## Phase 7 - First Combat Loop

Build:

- one enemy
- one weapon
- basic damage/death
- one wave loop
- minimal HUD

Exit when:

- REAP starts to feel like a game, not just a runtime

## Phase 8 - Custom Formats and Tools

Build:

- runtime model requirements
- `rmdl` format design
- runtime model loading path
- `rpk` archive design when needed
- small purpose-built editor/tooling only when workflow pressure justifies it

Exit when:

- authored content has a defined production pipeline

## Phase 9 - VM and Game Script Path

Build:

- `rvm` runtime
- assembler
- engine-side VM bridge
- gameplay scripts running through the VM path

Exit when:

- gameplay starts moving into the intended script/runtime split

## Phase 10 - Product Growth

Build:

- more enemies
- more maps
- better menus/config
- performance work
- stability work

Exit when:

- the game is stable and content-complete enough to feel real

## Solo-developer rule

At any moment:

- one active phase
- one active subsystem
- one or two active files

That is how a project this large stays sane.

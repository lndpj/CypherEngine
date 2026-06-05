# CypherEngine Project Structure

This is the intended long-term CypherEngine repository structure.

`CypherEngine` is the engine runtime. The game name can stay separate once the actual game identity is locked.

## Target layout

```text
CypherEngine/
├── README.md
├── Makefile or build wrapper
├── src/
│   ├── CypherEngine/
│   │   ├── CypherCommon/
│   │   ├── CypherSystem/
│   │   ├── CypherFileSystem/
│   │   ├── CypherLog/
│   │   ├── CypherCommand/
│   │   ├── CypherCVar/
│   │   ├── CypherConfig/
│   │   ├── CypherHost/
│   │   ├── CypherMath/
│   │   └── CypherRender/
│   ├── CypherGame/
│   ├── CypherClient/
│   ├── CypherServer/
│   ├── CypherNetwork/
│   ├── CypherPhysics/
│   ├── CypherAudio/
│   ├── CypherECS/
│   └── CypherScript/
├── rvm/
├── game/
├── tools/
├── data/
├── config/
├── thirdparty/
└── docs/
```

## Meaning of each top-level directory

- `src/CypherEngine`
  - native CypherEngine runtime
- `src/CypherGame`
  - native game/runtime bridge code when needed
- `src/CypherClient`
  - local player, prediction, presentation, HUD, input bridge
- `src/CypherServer`
  - authoritative simulation and multiplayer/session ownership
- `src/CypherNetwork`
  - packet, channel, socket, serialization, and transport code
- `src/CypherPhysics`
  - movement, traces, collision, and simulation helpers
- `src/CypherAudio`
  - sound runtime and audio resource playback
- `src/CypherECS`
  - entity/component storage and query layer
- `src/CypherScript`
  - VM/script bridge inside the native runtime
- `rvm`
  - standalone Cypher VM project
- `game`
  - gameplay scripts intended to run on the VM
- `tools`
  - standalone asset and pipeline tools
- `data`
  - runtime assets
- `config`
  - default cfg files
- `thirdparty`
  - vendored external libraries
- `docs`
  - architecture and process documentation

## Important note about the current repo

The current source already lives under `src/CypherEngine/`, with CryEngine-style subsystem folders such as `CypherRender`, `CypherSystem`, and `CypherFileSystem`.

Future migrations should add new `Cypher*` modules beside the existing runtime modules instead of inventing a different architecture each time.

## Key architectural message

This project is not just one executable.

It is a family of connected systems:
- native engine runtime
- scripting VM
- gameplay layer
- tools pipeline
- asset/data tree

That is why the structure must be explicit.

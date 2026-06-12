# CypherEngine Roadmap

This roadmap is the high-level summary version of [development_phases.md](development_phases.md).
For the full dated schedule, LOC ranges, subsystem acceptance criteria, and editor/toolchain plan, read [master_plan.md](master_plan.md).

`CypherEngine` is the engine runtime inside the CypherEngine project.

## The order that matters

1. finish the engine foundation and public contracts
2. finish the command/cvar/cfg backbone
3. finish memory arenas, pools, and allocation diagnostics
4. strengthen the virtual filesystem
5. build `CypherResource` as the asset lifetime layer
6. bootstrap the `SDL3`/`OpenGL` local 3D runtime path
7. add input and camera control
8. add materials, textures, meshes, and real renderer submission
9. design `CypherWorld` and the first custom map/world source format
10. design honest multiplayer/client-server shape
11. add custom tools and formats for models, materials, packages, maps, and resources
12. add VM/game-script path
13. grow `CypherEditor` into the long-term Qt editor application
14. push toward a stable product

## Scope reminder

REAP is not just:

- a shooter

It is also:

- an engine runtime
- a VM
- a tools pipeline
- a data/format ecosystem
- an editor/runtime workflow

That means progress must be staged carefully.

## Practical solo rule

At any given time:

- one active milestone
- one active subsystem focus
- one or two active files

That rule matters more than ambition.

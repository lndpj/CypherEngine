# CypherEngine Roadmap

This roadmap is the high-level summary version of [development_phases.md](/Users/karlosiric/Documents/MyProjects/REAP/docs/development_phases.md).

`CypherEngine` is the engine runtime inside the CypherEngine project.

## The order that matters

1. finish the engine foundation and public contracts
2. finish the command/cvar/cfg backbone
3. build the virtual filesystem
4. bootstrap the `SDL3` platform layer
5. bootstrap the `OpenGL` renderer path
6. get a local graybox 3D loop running
7. build `Quake III BSP` runtime support
8. design honest multiplayer/client-server shape
9. add custom tools and formats such as `rmdl`, `rpk`, and a small editor
10. add VM/game-script path
11. push toward a stable product

## Scope reminder

REAP is not just:

- a shooter

It is also:

- an engine runtime
- a VM
- a tools pipeline
- a data/format ecosystem

That means progress must be staged carefully.

## Practical solo rule

At any given time:

- one active milestone
- one active subsystem focus
- one or two active files

That rule matters more than ambition.

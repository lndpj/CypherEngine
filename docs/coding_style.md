# CypherEngine Coding Style

REAP is written in C++20, but the code style should stay close to disciplined C-style engine code.

## Style principles

- explicit ownership
- explicit data flow
- narrow module boundaries
- free functions over class hierarchies
- low-magic C++
- predictable runtime cost

## Naming direction

REAP follows Quake-style subsystem prefixes adapted to this project.

Examples:

- `com_` for common
- `r_` for renderer
- `sv_` for server
- `cl_` for client
- `net_` for network
- `bsp_` for BSP
- `pm_` for physics/movement
- `snd_` for audio
- `ecs_` for ECS integration
- `vm_` for engine/VM bridge
- `sys_` for platform
- `g_` for gameplay script modules

## Type style

- engine-facing types: `snake_case_t`
- constants/macros: `SCREAMING_SNAKE_CASE`
- file names: subsystem-prefixed and lowercase

## Documentation rule

Public engine-facing headers should document:
- what the type/function is for
- ownership/lifetime assumptions
- whether a helper allocates or returns a pointer into existing memory
- whether a time source is wall-clock or monotonic

## C++ features to prefer

- `std::array`
- `std::span`
- `std::string_view`
- `std::vector` where ownership is clear
- RAII in narrow, explicit cases

## C++ features to use sparingly

- exceptions
- RTTI
- heavy templates
- metaprogramming
- abstraction layers that hide runtime cost

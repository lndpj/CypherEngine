# Build Guide

## Current build path

The repository currently builds through CMake.

`CypherEngine` currently builds as the `cypherengine` executable target.

```bash
cmake -S . -B build
cmake --build build
```

Current executable:

```bash
./build/bin/cypherengine
```

## Planned convenience layer

A top-level `build.sh` will be introduced as a thin wrapper so the day-to-day build flow stays simple over the life of the project.

That script should remain:
- thin
- explicit
- a wrapper around the real build system

It should not replace the real build configuration.

## Long-term build picture

The intended full project has multiple build bodies:

- engine runtime
- standalone `rvm`
- game scripts
- tools

That means the eventual top-level build flow must account for:
- runtime compilation
- VM compilation
- script compilation
- asset pipeline invocation

## Current rule

Use the simplest build path that supports the current milestone.

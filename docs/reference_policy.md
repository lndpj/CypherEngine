# Engine Reference Policy

CypherEngine is structurally inspired by classic engine architecture, especially
idTech/Quake, GoldSrc/Source, and early CryEngine.

That inspiration is allowed and intentional.

## What is allowed

- studying subsystem boundaries
- studying gameplay/runtime architecture
- studying editor/runtime/tool separation
- studying naming conventions
- studying public interface layers
- studying patterns like:
  - command buffer
  - cvars
  - snapshots
  - BSP handling
  - VM boundaries
  - client/server split
  - resource compilers
  - world/map loading
  - editor object/property workflows
- re-deriving systems in original CypherEngine code

## What is not allowed by default

- copying source files wholesale
- rename-only ports
- blending copied code into original modules without provenance
- assuming license concerns disappear because the architecture is similar
- copying leaked/proprietary code or assets

## Working rule

We can be heavily inspired by the shape and discipline of classic engines.

We still write CypherEngine’s implementation ourselves unless a later deliberate
decision is documented and license-reviewed.

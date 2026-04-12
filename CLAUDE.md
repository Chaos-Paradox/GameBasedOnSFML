# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

This is a co-op action game built with C++20 and SFML 3.x, using an **Entity-Component-System (ECS)** architecture. The codebase is split into two projects:

- **Project1** — Core ECS framework (EntityManager, ComponentManager, EventBus)
- **Project2** — Game logic (pure C++, render-agnostic) with systems for combat, loot, AI, etc.

The game logic is deliberately decoupled from rendering — Project2 uses custom `Vec2`/`Rect` math types and communicates with the render layer via `RenderEventComponent`.

## Build Commands

### macOS (build with SFML + VisualSandbox)

```bash
cd Project2
cmake -S . -B build -DENABLE_SFML=ON
cmake --build build
```

### Run the VisualSandbox

```bash
# From project root (material/ path is relative to CWD)
./Project2/build/bin/VisualSandbox
```

### Full top-level build (builds both Project1 + Project2)

```bash
mkdir -p build && cd build
cmake ..
make
```

## Running Tests

### GTest suite

```bash
cd Project2
cmake --build build_sfml --target loot_pipeline_test
./build_sfml/test/loot_pipeline_test
```

### Legacy standalone tests (in `Project2/test/`, all commented out currently)

See `Project2/tests/README.md` for details. The unit tests in `tests/unit/` are currently commented out in `tests/unit/CMakeLists.txt`.

## Architecture Overview

### Layered Architecture

```
Render Layer (SFML / ImGui)     ← reads RenderEventComponent
         ↓
Project2 Game Logic (pure C++)  ← no SFML headers allowed
  └─ System layer (stateless): StateMachineSystem, CollisionSystem, CombatSystem, etc.
  └─ Component layer (POD structs): Transform, Hitbox, Hurtbox, DamageTag, etc.
  └─ Math library (custom): Vec2, Rect — no SFML types
```

### Frame Pipeline (per-frame execution order)

1. `InputSystem` → writes `InputCommand`
2. `StateMachineSystem` → reads input, manages state transitions
3. `MovementSystem` → writes velocity
4. `CollisionSystem` → writes `DamageTag`
5. `CombatSystem` / `DamageSystem` → applies damage
6. `DeathSystem` → mounts `DeathTag`
7. `LootSpawnSystem` → spawns loot from dead entities
8. `MagnetSystem` → pulls loot toward player
9. `PickupSystem` → collects loot
10. `CleanupSystem` → destroys expired entities
11. Render (external)

### Key Design Rules (DO NOT VIOLATE)

1. **No SFML in Project2** — Use `Vec2`/`Rect` only. No `#include <SFML/...>` in game logic.
2. **Components are pure data** — No methods, only fields (POD structs).
3. **Systems communicate via Components/Tags** — Never call another System directly. Use tag-driven communication (e.g., `DamageTag`, `DeathTag`).
4. **Entity creation** — Use `World::createEntity()`, not `new`.
5. **Entity destruction** — Use deferred destruction queue, never `delete` in `System::update()`.
6. **No dynamic allocation in System update loops** — No `new`/`malloc` in hot paths.

### Input System (v3.1 Single-Track Override Slot)

Uses a single `ActionIntent pendingIntent` slot with a unified `intentTimer`. Last-input-wins semantics; timer pauses during Hurt/Dead/Dash states ("time freeze magic").

### Key Files

| File | Purpose |
|------|---------|
| `Project2/include/core/ECS.h` | ECS core definitions |
| `Project2/include/core/GameWorld.h` | World/entity management |
| `Project2/include/components/` | All component definitions |
| `Project2/include/systems/` | All system definitions |
| `Project2/src/tools.cpp` | Main source file |
| `Project2/tests/sandbox/VisualSandbox.cpp` | Interactive test harness |
| `Project2/docs/00_ARCHITECTURE.md` | Full architecture document |

## Documentation

- Architecture: `Project2/docs/00_ARCHITECTURE.md`
- Data schema: `Project2/docs/01_DATA_SCHEMA.md`
- Feature docs: `Project2/docs/features/` (F001-StateMachine, F103-Attack, F201-Collision, etc.)
- Project1 ECS: `Project1/docs/00_ARCHITECTURE.md`

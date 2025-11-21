#include <iostream>
#include <vector>

#include "core/ECS.h"

#include "components/Position.h"
#include "components/Stats.h"
#include "components/InputCommand.h"
#include "components/AttackState.h"

#include "systems/InputSystem.h"
#include "systems/MovementSystem.h"
#include "systems/AttackSystem.h"
#include "systems/CombatSystem.h"
#include "utils/Logging.h"

// -----------------------------
// main: create 2 players, simulate frames, print logs
// -----------------------------
int main() {
    ECS ecs;

    ComponentStore<Position> positions;
    ComponentStore<Stats> stats;
    ComponentStore<InputCommand> inputs;
    ComponentStore<AttackState> attacks;

    InputSystem  inputSys;
    MovementSystem moveSys;
    AttackSystem atkSys;
    CombatSystem combatSys;

    // Create two player entities
    Entity p1 = ecs.create();
    Entity p2 = ecs.create();

    // Initialize components
    positions.add(p1, Position{ 0.0f, 0.0f });
    positions.add(p2, Position{ 1.0f, 0.0f }); // inside 2.0 distance, in front (x=1)

    stats.add(p1, Stats{ 100.0f, 10.0f, 1.0f }); // hp, dmg, atkspd
    stats.add(p2, Stats{ 100.0f, 12.0f, 1.5f });

    inputs.add(p1, InputCommand{ Command::None });
    inputs.add(p2, InputCommand{ Command::None });

    attacks.add(p1, AttackState{ 0.0f, false });
    attacks.add(p2, AttackState{ 0.0f, false });

    std::vector<Entity> players = { p1, p2 };

    // Simulation parameters
    const int ticks = 60;

    const float dt = 1.0f / ticks; // 0.1s per frame
    std::cout << "dt = " << dt << std::endl;
    const int frames = 300;

    // We will command both players to Attack every frame to show cooldown behavior.
    for (int f = 0; f < frames; ++f) {
        // set input (in real game, this comes from keyboard / network)
        inputSys.setCommand(inputs, p1, Command::Attack);
        inputSys.setCommand(inputs, p2, Command::Attack);

        // update attack system (sets attacking flag when attack triggered)
        atkSys.update(inputs, attacks, stats, p1, dt);
        atkSys.update(inputs, attacks, stats, p2, dt);

        // movement (not used in this tight demo, but left for completeness)
        moveSys.update(positions, inputs, p1);
        moveSys.update(positions, inputs, p2);

        // combat resolution
        combatSys.update(positions, attacks, stats, players);

        // debug print
        printFrameDebug(ecs, positions, stats, inputs, attacks, f);

        // small: clear one-frame-only commands (simulate key press vs hold)
        // here we keep commands as Attack to show repeated attempts; if you want single-tap,
        // you can set inputs.get(p1).cmd = Command::None after processing.
    }

    std::cout << "\n[FINAL] Remaining HP: Entity " << p1 << " = " << stats.get(p1).hp
        << " | Entity " << p2 << " = " << stats.get(p2).hp << "\n";

    return 0;
}

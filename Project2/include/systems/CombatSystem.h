#pragma once
#include <vector>
#include "../components/Position.h"
#include "../components/AttackState.h"
#include "../components/Stats.h"
#include "../utils/Math.h"

class CombatSystem {
public:
    // entities: list of entities to consider as possible targets (players)
    // we assume attacker faces +x; sector check is simple
    void update(ComponentStore<Position>& pos,
        ComponentStore<AttackState>& atk,
        ComponentStore<Stats>& stats,
        const std::vector<Entity>& entities)
    {
        // collect attacks this frame
        for (auto a : entities) {
            if (!atk.has(a) || !atk.get(a).attacking) continue;
            if (!pos.has(a)) continue;
            // attacker 'a' attacks; check others
            for (auto b : entities) {
                if (a == b) continue;
                if (!pos.has(b) || !stats.has(b) || !stats.has(a)) continue;
                if (inSector(pos.get(a).x, pos.get(a).y, pos.get(b).x, pos.get(b).y)) {
                    float dmg = stats.get(a).attackDamage;
                    stats.get(b).hp -= dmg;
                    std::cout << "[COMBAT] Entity " << a << " hit Entity " << b
                        << " for " << dmg << " dmg. (HP now " << stats.get(b).hp << ")\n";
                }
                else {
                    std::cout << "[COMBAT] Entity " << a << " attacked but target " << b << " is out of sector/distance.\n";
                }
            }
        }
    }
};

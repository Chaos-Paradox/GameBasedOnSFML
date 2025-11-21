#pragma once
#include "../components/InputCommand.h"
#include "../components/AttackState.h"
#include "../components/Stats.h"

class AttackSystem {
public:
    // If command==Attack and cooldown <= 0 -> trigger attack, set cooldown based on attackSpeed
    void update(ComponentStore<InputCommand>& inputs,
        ComponentStore<AttackState>& atk,
        ComponentStore<Stats>& stats,
        Entity e,
        float dt)
    {
        if (!inputs.has(e) || !atk.has(e) || !stats.has(e)) return;
        auto& ist = inputs.get(e);
        auto& st = atk.get(e);
        auto& ss = stats.get(e);

        // reduce cooldown
        if (st.cooldown > 0.0f) {
            st.cooldown -= dt;
            if (st.cooldown < 0.0f) st.cooldown = 0.0f;
        }
        st.attacking = false;

        if (ist.cmd == Command::Attack && st.cooldown <= 0.0f) {
            st.attacking = true;
            st.cooldown = 1.0f / std::max(0.0001f, ss.attackSpeed);
            // NOTE: we keep attacking flag true only for this frame; CombatSystem will process it
        }
    }
};

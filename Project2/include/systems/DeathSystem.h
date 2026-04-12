#pragma once
#include "core/GameWorld.h"

/**
 * @brief 死亡处理系统
 */
class DeathSystem {
public:
    void update(GameWorld& world, float dt)
    {
        (void)dt;

        auto entities = world.deathTags.entityList();
        for (Entity entity : entities) {
            bool isPlayer = world.evolutions.has(entity);

            if (world.states.has(entity)) {
                auto& state = world.states.get(entity);
                state.currentState = CharacterState::Dead;
                state.previousState = CharacterState::Dead;
                state.stateTimer = 0.0f;
            }

            if (isPlayer) {
                std::cout << "[DeathSystem] Player died! Immortal, skipping destruction.\n";
                world.deathTags.remove(entity);
                continue;
            }

            std::cout << "[DeathSystem] Entity " << entity << " marked for cleanup.\n";
        }
    }
};

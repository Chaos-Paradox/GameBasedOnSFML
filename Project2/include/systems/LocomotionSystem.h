#pragma once
#include "core/GameWorld.h"
#include <cmath>

/**
 * @brief 位移系统（速度计算）
 */
class LocomotionSystem {
public:
    void update(GameWorld& world, float dt)
    {
        (void)dt;

        auto entities = world.states.entityList();
        for (Entity entity : entities) {
            const auto& state = world.states.get(entity);

            if (!world.transforms.has(entity)) continue;

            if (state.currentState == CharacterState::Dash ||
                state.currentState == CharacterState::Attack ||
                state.currentState == CharacterState::Hurt ||
                state.currentState == CharacterState::Dead ||
                state.currentState == CharacterState::KnockedAirborne) {
                continue;
            }

            auto& transform = world.transforms.get(entity);

            if (!world.characters.has(entity)) continue;
            const auto& character = world.characters.get(entity);
            if (!world.inputs.has(entity)) continue;
            const auto& input = world.inputs.get(entity);

            Vec2 dir = input.moveDir;

            if (dir.x != 0.0f || dir.y != 0.0f) {
                transform.facingX = dir.x;
                transform.facingY = dir.y;

                float length = std::sqrt(dir.x * dir.x + dir.y * dir.y);
                if (length > 0.0f) {
                    dir.x /= length;
                    dir.y /= length;
                }

                transform.velocity.x = dir.x * character.baseMoveSpeed;
                transform.velocity.y = dir.y * character.baseMoveSpeed;
            } else {
                transform.velocity = {0.0f, 0.0f};
            }
        }
    }
};

#pragma once
#include "core/GameWorld.h"
#include <cmath>

/**
 * @brief 位移执行系统（将速度化为实质距离）
 */
class MovementSystem {
public:
    void update(GameWorld& world, float dt)
    {
        auto entities = world.transforms.entityList();
        for (Entity entity : entities) {
            auto& transform = world.transforms.get(entity);

            bool shouldApplyFriction = true;
            bool isAirborne = false;

            if (world.zTransforms.has(entity)) {
                isAirborne = (world.zTransforms.get(entity).z > 0.0f);
            }

            if (world.states.has(entity)) {
                auto currentState = world.states.get(entity).currentState;
                if (currentState == CharacterState::Move ||
                    currentState == CharacterState::Dash ||
                    currentState == CharacterState::Attack) {
                    shouldApplyFriction = false;
                }
            }

            if (world.bombs.has(entity) && world.zTransforms.has(entity)) {
                if (world.zTransforms.get(entity).z > 5.0f) {
                    shouldApplyFriction = false;
                }
            }

            if (shouldApplyFriction) {
                if (!isAirborne) {
                    float friction = std::pow(0.001f, dt);
                    transform.velocity.x *= friction;
                    transform.velocity.y *= friction;

                    if (std::abs(transform.velocity.x) < 5.0f) transform.velocity.x = 0.0f;
                    if (std::abs(transform.velocity.y) < 5.0f) transform.velocity.y = 0.0f;
                } else {
                    transform.velocity.x *= 0.99f;
                    transform.velocity.y *= 0.99f;
                }
            }

            // 速度上限：防止高速实体隧穿（普通实体遵守，投掷物和动量实体免限速）
            constexpr float MAX_SPEED = 1800.0f;
            float speed = std::sqrt(transform.velocity.x * transform.velocity.x + transform.velocity.y * transform.velocity.y);

            bool isHighSpeedProjectile = false;
            if (world.throwables.has(entity)) {
                isHighSpeedProjectile = true;
            }
            if (world.momentums.has(entity)) {
                isHighSpeedProjectile = true;
            }

            if (!isHighSpeedProjectile && speed > MAX_SPEED) {
                float invSpeed = 1.0f / speed;
                transform.velocity.x *= MAX_SPEED * invSpeed;
                transform.velocity.y *= MAX_SPEED * invSpeed;
            }

            transform.position.x += transform.velocity.x * dt;
            transform.position.y += transform.velocity.y * dt;
        }
    }
};

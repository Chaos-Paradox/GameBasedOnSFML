#pragma once
#include "core/GameWorld.h"
#include <cmath>
#include <iostream>

/**
 * @brief 冲刺系统
 */
class DashSystem {
public:
    void update(GameWorld& world, float dt)
    {
        auto dashEntities = world.dashes.entityList();
        for (Entity entity : dashEntities) {
            if (!world.dashes.has(entity) || !world.states.has(entity) || !world.transforms.has(entity)) {
                continue;
            }

            auto& dash = world.dashes.get(entity);
            auto& state = world.states.get(entity);
            auto& transform = world.transforms.get(entity);

            if (state.currentState != CharacterState::Dash &&
                state.currentState != CharacterState::Hurt &&
                state.currentState != CharacterState::Dead) {

                bool hasDashIntent = world.inputs.has(entity) &&
                                     world.inputs.get(entity).pendingIntent == ActionIntent::Dash &&
                                     world.inputs.get(entity).intentTimer > 0.0f;

                if (hasDashIntent && dash.cooldownTimer <= 0.0f) {
                    if (transform.facingX != 0.0f || transform.facingY != 0.0f) {
                        float length = std::sqrt(transform.facingX * transform.facingX +
                                                transform.facingY * transform.facingY);
                        if (length > 0.0f) {
                            dash.dashDir.x = transform.facingX / length;
                            dash.dashDir.y = transform.facingY / length;
                        } else {
                            dash.dashDir = {1.0f, 0.0f};
                        }
                    } else {
                        dash.dashDir = {1.0f, 0.0f};
                    }

                    state.currentState = CharacterState::Dash;
                    state.previousState = CharacterState::Dash;

                    transform.velocity = dash.dashDir * 2000.0f;

                    dash.dashTimer = dash.dashDuration;
                    dash.iframeTimer = dash.iframeDuration;
                    dash.isInvincible = true;
                    dash.cooldownTimer = dash.cooldown;

                    if (world.inputs.has(entity)) {
                        world.inputs.get(entity).pendingIntent = ActionIntent::None;
                        world.inputs.get(entity).intentTimer = 0.0f;
                    }

                    std::cout << "[DashSystem] ✓ DASH! velocity=2000 dir=("
                              << dash.dashDir.x << ", " << dash.dashDir.y << ")\n";
                }
            }

            if (state.currentState == CharacterState::Dash) {
                dash.dashTimer -= dt;
                if (dash.iframeTimer > 0.0f) {
                    dash.iframeTimer -= dt;
                    if (dash.iframeTimer <= 0.0f) {
                        dash.isInvincible = false;
                    }
                }

                float friction = std::pow(0.1f, dt);
                transform.velocity.x *= friction;
                transform.velocity.y *= friction;

                std::cout << "[DashSystem] Dashing! velocity=(" << transform.velocity.x
                          << ", " << transform.velocity.y << ") friction=" << friction << "\n";

                bool speedTooLow = (std::abs(transform.velocity.x) < 50.0f &&
                                   std::abs(transform.velocity.y) < 50.0f);

                if (dash.dashTimer <= 0.0f || speedTooLow) {
                    state.currentState = CharacterState::Idle;
                    state.previousState = CharacterState::Idle;
                    transform.velocity = {0.0f, 0.0f};
                    dash.isInvincible = false;

                    std::cout << "[DashSystem] Dash finished!\n";
                }
            } else {
                if (dash.cooldownTimer > 0.0f) {
                    dash.cooldownTimer -= dt;
                }
            }
        }
    }
};

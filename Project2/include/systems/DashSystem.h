#pragma once
#include "core/GameWorld.h"
#include <cmath>
#include <iostream>

/**
 * @brief 冲刺系统
 *
 * 速度曲线设计：
 *   1. 无敌帧期间（iframeTimer > 0）：保持 dashSpeed 最高速度，不衰减
 *   2. 无敌帧结束后（iframeTimer <= 0）：按指数摩擦衰减
 *   3. dashTimer 归零后进入后摇（Recovery），期间无法输入
 *   4. 后摇结束后回到 Idle
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

            // ========== 启动 Dash ==========
            // 注意：非 Dash/Recovery/Hurt/Dead 状态下才可触发新 Dash
            if (state.currentState != CharacterState::Dash &&
                state.currentState != CharacterState::Recovery &&
                state.currentState != CharacterState::Hurt &&
                state.currentState != CharacterState::Dead) {

                bool hasDashIntent = world.inputs.has(entity) &&
                                     world.inputs.get(entity).pendingIntent == ActionIntent::Dash &&
                                     world.inputs.get(entity).intentTimer > 0.0f;

                if (hasDashIntent && dash.cooldownTimer <= 0.0f) {
                    // 计算冲刺方向
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

                    // 启动时设置最高速度
                    transform.velocity = dash.dashDir * dash.dashSpeed;

                    dash.dashTimer = dash.dashDuration;
                    dash.iframeTimer = dash.iframeDuration;
                    dash.isInvincible = true;
                    dash.cooldownTimer = dash.cooldown;

                    if (world.inputs.has(entity)) {
                        world.inputs.get(entity).pendingIntent = ActionIntent::None;
                        world.inputs.get(entity).intentTimer = 0.0f;
                    }

                    std::cout << "[DashSystem] DASH! speed=" << dash.dashSpeed
                              << " dir=(" << dash.dashDir.x << ", " << dash.dashDir.y << ")\n";
                }
            }

            // ========== Dash 状态更新 ==========
            if (state.currentState == CharacterState::Dash) {
                dash.dashTimer -= dt;

                // 无敌帧计时
                if (dash.iframeTimer > 0.0f) {
                    dash.iframeTimer -= dt;
                    if (dash.iframeTimer <= 0.0f) {
                        dash.isInvincible = false;
                    }
                }

                // 速度曲线：无敌帧期间保持最高速度，无敌帧结束后指数衰减
                if (dash.iframeTimer > 0.0f) {
                    // 保持最高速度不变
                    transform.velocity = dash.dashDir * dash.dashSpeed;
                } else {
                    // 指数摩擦衰减
                    float friction = std::pow(0.1f, dt);
                    transform.velocity.x *= friction;
                    transform.velocity.y *= friction;
                }

                std::cout << "[DashSystem] Dashing! velocity=(" << transform.velocity.x
                          << ", " << transform.velocity.y << ") iframe=" << dash.iframeTimer << "\n";

                // Dash 结束条件：计时归零 或 速度过低
                bool speedTooLow = (std::abs(transform.velocity.x) < 50.0f &&
                                   std::abs(transform.velocity.y) < 50.0f);

                if (dash.dashTimer <= 0.0f || speedTooLow) {
                    // 进入后摇阶段
                    state.currentState = CharacterState::Recovery;
                    state.previousState = CharacterState::Recovery;
                    dash.recoveryTimer = dash.recoveryDuration;

                    // 后摇开始时速度清零
                    transform.velocity = {0.0f, 0.0f};

                    std::cout << "[DashSystem] Dash finished, entering Recovery ("
                              << dash.recoveryDuration << "s)\n";
                }
            }

            // ========== Recovery 状态更新 ==========
            if (state.currentState == CharacterState::Recovery) {
                dash.recoveryTimer -= dt;

                if (dash.recoveryTimer <= 0.0f) {
                    state.currentState = CharacterState::Idle;
                    state.previousState = CharacterState::Idle;
                    dash.recoveryTimer = 0.0f;

                    std::cout << "[DashSystem] Recovery finished, back to Idle\n";
                }
            }

            // ========== 非 Dash 状态下冷却计时 ==========
            if (state.currentState != CharacterState::Dash) {
                if (dash.cooldownTimer > 0.0f) {
                    dash.cooldownTimer -= dt;
                }
            }
        }
    }
};

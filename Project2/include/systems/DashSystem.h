#pragma once
#include "core/GameWorld.h"
#include <cmath>
#include <iostream>

/**
 * @brief 冲刺系统（充能模式）
 *
 * 充能设计：
 *   1. 最多存储 maxCharges 次充能（默认 2）
 *   2. 每次 Dash 消耗 1 次充能
 *   3. 充能按 rechargeCooldown 自动恢复
 *   4. 充能未满时持续恢复，最多到 maxCharges
 *
 * 速度曲线：
 *   1. 无敌帧期间（iframeTimer > 0）：保持 dashSpeed 最高速度，不衰减
 *   2. 无敌帧结束后（iframeTimer <= 0）：按指数摩擦衰减
 *   3. dashTimer 归零后直接回到 Idle（无后摇）
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

                // 充能模式：currentCharges > 0 才能启动
                if (hasDashIntent && dash.currentCharges > 0) {
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

                    // 消耗 1 次充能
                    dash.currentCharges--;
                    // 如果充能未满，开始充能计时
                    if (dash.currentCharges < dash.maxCharges) {
                        dash.rechargeTimer = dash.rechargeCooldown;
                    }

                    if (world.inputs.has(entity)) {
                        world.inputs.get(entity).pendingIntent = ActionIntent::None;
                        world.inputs.get(entity).intentTimer = 0.0f;
                    }

                    std::cout << "[DashSystem] DASH! speed=" << dash.dashSpeed
                              << " dir=(" << dash.dashDir.x << ", " << dash.dashDir.y
                              << ") charges=" << dash.currentCharges << "/" << dash.maxCharges << "\n";
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
                    // 无后摇时直接回到 Idle，否则进入 Recovery
                    if (dash.recoveryDuration <= 0.0f) {
                        state.currentState = CharacterState::Idle;
                        state.previousState = CharacterState::Idle;
                        std::cout << "[DashSystem] Dash finished, back to Idle (no recovery)\n";
                    } else {
                        state.currentState = CharacterState::Recovery;
                        state.previousState = CharacterState::Recovery;
                        dash.recoveryTimer = dash.recoveryDuration;

                        std::cout << "[DashSystem] Dash finished, entering Recovery ("
                                  << dash.recoveryDuration << "s)\n";
                    }

                    // Dash 结束时速度清零
                    transform.velocity = {0.0f, 0.0f};
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

            // ========== 充能恢复 ==========
            // 非 Dash 状态下，如果充能未满则进行充能
            if (state.currentState != CharacterState::Dash &&
                dash.currentCharges < dash.maxCharges) {
                dash.rechargeTimer -= dt;

                if (dash.rechargeTimer <= 0.0f) {
                    dash.currentCharges++;
                    std::cout << "[DashSystem] Charge recovered! charges="
                              << dash.currentCharges << "/" << dash.maxCharges << "\n";

                    // 如果还没满，重置计时器继续充能
                    if (dash.currentCharges < dash.maxCharges) {
                        dash.rechargeTimer = dash.rechargeCooldown;
                    } else {
                        dash.rechargeTimer = 0.0f;
                    }
                }
            }
        }
    }
};

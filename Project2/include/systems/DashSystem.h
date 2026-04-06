#pragma once
#include "../core/Component.h"
#include "../core/ECS.h"
#include "../components/DashComponent.h"
#include "../components/StateMachine.h"
#include "../components/Transform.h"
#include <cmath>
#include <iostream>

/**
 * @brief 冲刺系统
 * 
 * ⚠️ 关键设计：初速度只在切入 Dash 状态的那一帧设置！
 * ⚠️ Fixed Timestep 适配：dashPressed 在循环外检测，避免多次触发
 */
class DashSystem {
public:
    void update(
        ComponentStore<DashComponent>& dashes,
        ComponentStore<StateMachineComponent>& states,
        ComponentStore<TransformComponent>& transforms,
        bool dashPressed,  // ← 冲刺按键标志（在循环外检测，只触发一次）
        float dt)
    {
        auto dashEntities = dashes.entityList();
        for (Entity entity : dashEntities) {
            if (!dashes.has(entity) || !states.has(entity) || !transforms.has(entity)) {
                continue;
            }
            
            auto& dash = dashes.get(entity);
            auto& state = states.get(entity);
            auto& transform = transforms.get(entity);
            
            // ← 1. 检测冲刺输入（只在非 Dash 状态检测）
            if (state.currentState != CharacterState::Dash && 
                state.currentState != CharacterState::Hurt &&
                state.currentState != CharacterState::Dead) {
                
                // ← 使用传入的 dashPressed 标志（只触发一次）
                if (dashPressed && dash.cooldownTimer <= 0.0f) {
                    // 获取 facing 方向
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
                    
                    // 切换为 Dash 状态
                    state.currentState = CharacterState::Dash;
                    state.previousState = CharacterState::Dash;
                    
                    // ← 【关键】初速度只在这里设置一次！
                    transform.velocity = dash.dashDir * 2000.0f;  // 初速度 2000
                    
                    // 设置冲刺标志
                    dash.dashTimer = dash.dashDuration;
                    dash.iframeTimer = dash.iframeDuration;
                    dash.isInvincible = true;
                    dash.cooldownTimer = dash.cooldown;
                    
                    std::cout << "[DashSystem] ✓ DASH! velocity=2000 dir=(" 
                              << dash.dashDir.x << ", " << dash.dashDir.y << ")\n";
                }
            }
            
            // ← 2. 处理 Dash 状态（只做衰减，不重置速度！）
            if (state.currentState == CharacterState::Dash) {
                // 减少计时器
                dash.dashTimer -= dt;
                if (dash.iframeTimer > 0.0f) {
                    dash.iframeTimer -= dt;
                    if (dash.iframeTimer <= 0.0f) {
                        dash.isInvincible = false;
                    }
                }
                
                // ← 【核心】基于时间的指数衰减（只衰减，不重置！）
                // pow(0.1f, dt) = 约 0.955 (dt=0.0166667)
                // 每帧衰减 4.5%，更平滑
                float friction = std::pow(0.1f, dt);
                transform.velocity.x *= friction;
                transform.velocity.y *= friction;
                
                std::cout << "[DashSystem] Dashing! velocity=(" << transform.velocity.x 
                          << ", " << transform.velocity.y << ") friction=" << friction << "\n";
                
                // 冲刺结束：时间到 或 速度足够小
                bool speedTooLow = (std::abs(transform.velocity.x) < 50.0f && 
                                   std::abs(transform.velocity.y) < 50.0f);
                
                if (dash.dashTimer <= 0.0f || speedTooLow) {
                    // ← 【关键】释放路权，回到 Idle 状态
                    state.currentState = CharacterState::Idle;
                    state.previousState = CharacterState::Idle;
                    transform.velocity = {0.0f, 0.0f};
                    dash.isInvincible = false;
                    
                    std::cout << "[DashSystem] Dash finished!\n";
                }
            } else {
                // ← 3. 非 Dash 状态：更新冷却
                if (dash.cooldownTimer > 0.0f) {
                    dash.cooldownTimer -= dt;
                }
            }
        }
    }
};

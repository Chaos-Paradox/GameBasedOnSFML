#pragma once
#include "../core/Component.h"
#include "../core/ECS.h"
#include "../components/DamageEventComponent.h"
#include "../components/DeathTag.h"
#include "../components/Character.h"
#include "../components/StateMachine.h"
#include "../components/DashComponent.h"
#include "../components/Transform.h"
#include "../components/ZTransformComponent.h"
#include "../components/DamageTextComponent.h"
#include "../components/Lifetime.h"
#include <iostream>
#include <cstdlib>

/**
 * @brief GameJuice 全局打击感状态器
 * 
 * 控制时间流速（顿帧 Hit-Stop）和屏幕震动（Camera Shake）
 * 由 DamageSystem 触发，由 VisualSandbox 主循环消费
 */
struct GameJuice {
    float timeScale{1.0f};         // 游戏世界时间流速
    float hitStopTimer{0.0f};      // 顿帧剩余时间（真实时间）
    float shakeTimer{0.0f};        // 震动剩余时间（真实时间）
    float shakeIntensity{0.0f};    // 当前震动强度（像素偏移量）
};

/**
 * @brief 伤害结算系统（纯粹的执行者）
 * 
 * 职责：
 * - 扫描所有 DamageEventComponent 事件实体
 * - 读取 actualDamage 并扣减 CharacterComponent.currentHP
 * - HP ≤ 0 时挂载 DeathTag（死亡通知单）
 * - 不销毁事件实体（由 CleanupEventSystem 统一处理）
 * 
 * ⚠️ 关键设计：只负责执行，不负责清理
 * 
 * @see DamageEventComponent - 伤害事件载荷
 * @see CleanupEventSystem - 事件实体清理
 */
class DamageSystem {
public:
    void update(
        ComponentStore<CharacterComponent>& characters,
        ComponentStore<DamageEventComponent>& damageEvents,
        ComponentStore<DeathTag>& deathTags,
        ComponentStore<StateMachineComponent>& states,
        const ComponentStore<DashComponent>& dashes,
        ComponentStore<TransformComponent>& transforms,
        ComponentStore<ZTransformComponent>& zTransforms,
        ComponentStore<DamageTextComponent>& damageTexts,
        ComponentStore<LifetimeComponent>& lifetimes,
        ECS& ecs,
        GameJuice& juice)  // ← 打击感状态器
    {
        auto entities = damageEvents.entityList();
        for (Entity eventEntity : entities) {
            const auto& event = damageEvents.get(eventEntity);
            
            // 安全检查
            if (event.target == INVALID_ENTITY) {
                continue;
            }
            
            if (characters.has(event.target)) {
                // ← 【核心改动】检查是否处于冲刺无敌帧
                if (isInvincible(event.target, states, dashes)) {
                    std::cout << "[Combat] Dodge! Damage avoided during i-frames.\n";
                    continue;  // 跳过伤害结算
                }
                
                auto& character = characters.get(event.target);
                int oldHP = character.currentHP;
                character.currentHP -= event.actualDamage;
                
                // 死亡检测
                if (character.currentHP <= 0) {
                    character.currentHP = 0;
                    
                    // ← 【关键】贴上 DeathTag（死亡通知单）
                    if (!deathTags.has(event.target)) {
                        deathTags.add(event.target, {});
                        std::cout << "[DamageSystem] DeathTag added to Entity " << event.target << "\n";
                    }
                }
                
                // ← 【核心改动】输出暴击信息
                std::cout << "[DamageSystem] Entity " << event.target 
                          << " took " << event.actualDamage << " damage"
                          << (event.isCritical ? " [CRITICAL!]" : "")
                          << " (HP: " << oldHP << " -> " << character.currentHP << ")\n";
                
                // ← 【新增】生成伤害飘字实体（完全独立，不和物理挂钩）
                Entity textEntity = ecs.create();
                
                // 位置稍微向上偏移，带有轻微的随机数防止多个数字重叠
                float randomOffsetX = (static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX)) * 20.0f - 10.0f;
                
                transforms.add(textEntity, {
                    .position = {event.hitPosition.x + randomOffsetX, event.hitPosition.y - 40.0f},
                    .scale = {1.0f, 1.0f},
                    .rotation = 0.0f,
                    .velocity = {0.0f, 0.0f},
                    .facingX = 1.0f,
                    .facingY = 0.0f
                });
                
                damageTexts.add(textEntity, {
                    .text = std::to_string(event.actualDamage) + (event.isCritical ? "!" : ""),
                    .timer = 1.0f,
                    .position = {event.hitPosition.x + randomOffsetX, event.hitPosition.y - 40.0f},
                    .velocity = {0.0f, -50.0f},  // 向上飘动
                    .isCritical = event.isCritical,
                    .alpha = 1.0f,
                    .fontSize = event.isCritical ? 32.0f : 24.0f,
                    .fadeOutStart = 0.5f
                });
                
                lifetimes.add(textEntity, {
                    .timeLeft = 0.8f,
                    .autoDestroy = true
                });
                
                // 强制画在最高层（Z=50）
                zTransforms.add(textEntity, {
                    .z = 50.0f,
                    .vz = 0.0f,
                    .gravity = 0.0f,
                    .height = 10.0f
                });
                
                std::cout << "[DamageSystem] 📝 Created damage text! ID=" << (uint32_t)textEntity << " dmg=" << event.actualDamage << "\n";
                
                // ← 【新增】处理击飞效果
                if ((event.knockbackXY > 0.0f || event.knockbackZ > 0.0f) && 
                    states.has(event.target) && 
                    states.get(event.target).currentState != CharacterState::Dead) {
                    
                    auto& state = states.get(event.target);
                    
                    // 强制切换到击飞状态（覆盖一切动作，除了 Dead）
                    if (state.currentState != CharacterState::KnockedAirborne) {
                        state.currentState = CharacterState::KnockedAirborne;
                        state.previousState = CharacterState::KnockedAirborne;
                        std::cout << "[DamageSystem] 💨 击飞！Entity " << event.target 
                                  << " knockbackXY=" << event.knockbackXY 
                                  << " knockbackZ=" << event.knockbackZ << "\n";
                    }
                    
                    // 赋予击飞动量
                    if (transforms.has(event.target)) {
                        auto& transform = transforms.get(event.target);
                        transform.velocity.x = event.hitDirection.x * event.knockbackXY;
                        transform.velocity.y = event.hitDirection.y * event.knockbackXY;
                    }
                    
                    // 赋予 Z 轴起飞动量
                    if (zTransforms.has(event.target)) {
                        auto& zTrans = zTransforms.get(event.target);
                        zTrans.vz = event.knockbackZ;
                        zTrans.z += 5.0f;  // 强行拉离地面，防止立即判定落地
                    }
                }
                
                // ← 【GameJuice】打击感触发：根据伤害/击飞力度赋予不同级别的反馈
                if (event.knockbackXY > 1000.0f) {
                    // 核弹级打击感（炸弹爆炸、重击）
                    juice.hitStopTimer = 0.12f;      // 画面停滞 0.12 秒（夸张卡肉）
                    juice.timeScale = 0.0f;
                    juice.shakeTimer = 0.3f;          // 震动持续 0.3 秒
                    juice.shakeIntensity = 20.0f;     // 屏幕狂震 20 像素
                    std::cout << "[GameJuice] 💥 HEAVY hit-stop + shake!\n";
                } else if (event.actualDamage > 0) {
                    // 普通级打击感（剑气平 A）
                    juice.hitStopTimer = 0.04f;       // 停滞 2-3 帧
                    juice.timeScale = 0.0f;
                    juice.shakeTimer = 0.1f;
                    juice.shakeIntensity = 5.0f;      // 轻微震动
                    std::cout << "[GameJuice] ⚡ Light hit-stop + shake!\n";
                }
            }
            
            // ← 【核心修复】处理完事件后立即移除，防止下一帧重复处理！
            // 注意：不销毁实体 ID，由 CleanupSystem 统一处理
            // 但必须移除 DamageEventComponent，让下一帧扫描不到
            damageEvents.remove(eventEntity);
        }
    }
    
private:
    /**
     * @brief 检查实体是否处于无敌状态
     * 
     * @param entity 实体 ID
     * @param states StateMachineComponent 存储
     * @param dashes DashComponent 存储
     * @return true = 无敌，false = 可受伤
     */
    bool isInvincible(
        Entity entity,
        const ComponentStore<StateMachineComponent>& states,
        const ComponentStore<DashComponent>& dashes)
    {
        // 检查是否处于 Dash 状态
        if (states.has(entity)) {
            const auto& state = states.get(entity);
            if (state.currentState == CharacterState::Dash) {
                // 检查是否有 DashComponent 且处于无敌帧
                if (dashes.has(entity)) {
                    const auto& dash = dashes.get(entity);
                    if (dash.isInvincible && dash.iframeTimer > 0.0f) {
                        return true;  // 无敌帧中
                    }
                }
            }
        }
        
        return false;  // 不无敌
    }
};

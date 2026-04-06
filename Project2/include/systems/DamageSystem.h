#pragma once
#include "../core/Component.h"
#include "../components/DamageEventComponent.h"
#include "../components/DeathTag.h"
#include "../components/Character.h"
#include "../components/StateMachine.h"
#include "../components/DashComponent.h"
#include <iostream>

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
        const ComponentStore<DamageEventComponent>& damageEvents,  // ← 改为读取事件
        ComponentStore<DeathTag>& deathTags,
        const ComponentStore<StateMachineComponent>& states,      // ← 新增：检查状态
        const ComponentStore<DashComponent>& dashes)              // ← 新增：检查无敌帧
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
                        deathTags.add(event.target, {0.0f});
                        std::cout << "[DamageSystem] DeathTag added to Entity " << event.target << "\n";
                    }
                }
                
                // ← 【核心改动】输出暴击信息
                std::cout << "[DamageSystem] Entity " << event.target 
                          << " took " << event.actualDamage << " damage"
                          << (event.isCritical ? " [CRITICAL!]" : "")
                          << " (HP: " << oldHP << " -> " << character.currentHP << ")\n";
            }
            
            // ← 【核心改动】不再销毁事件实体（由 CleanupEventSystem 统一处理）
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

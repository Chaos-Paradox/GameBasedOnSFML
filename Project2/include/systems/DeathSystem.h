#pragma once
#include "../core/Component.h"
#include "../core/ECS.h"
#include "../components/StateMachine.h"
#include "../components/Transform.h"
#include "../components/Character.h"
#include "../components/Hurtbox.h"
#include "../components/LootDrop.h"
#include "../components/InputCommand.h"
#include "../components/DeathTag.h"
#include "../components/Evolution.h"

/**
 * @brief 死亡处理系统
 * 
 * 职责：
 * - 扫描所有 DeathTag
 * - 切换实体为 Dead 状态
 * - 【关键修复】玩家免疫销毁（通过 EvolutionComponent 判断）
 * - 小怪正常销毁
 */
class DeathSystem {
public:
    void update(
        ComponentStore<StateMachineComponent>& states,
        ComponentStore<TransformComponent>& transforms,
        ComponentStore<CharacterComponent>& characters,
        ComponentStore<HurtboxComponent>& hurtboxes,
        ComponentStore<LootDropComponent>& lootDrops,
        ComponentStore<InputCommand>& inputs,
        ComponentStore<DeathTag>& deathTags,
        ECS& ecs,
        ComponentStore<EvolutionComponent>& evolutions,  // ← 新增：用于判断玩家
        float dt)
    {
        (void)dt;
        
        auto entities = deathTags.entityList();
        for (Entity entity : entities) {
            // ← 【核心改动】判断是否为玩家（有 EvolutionComponent 的是玩家）
            bool isPlayer = evolutions.has(entity);
            
            // 切换为死亡状态
            if (states.has(entity)) {
                auto& state = states.get(entity);
                state.currentState = CharacterState::Dead;
                state.previousState = CharacterState::Dead;
                state.stateTimer = 0.0f;
            }
            
            if (isPlayer) {
                // ⚠️ 玩家免疫销毁！只切换状态，不挂 DeathTag，不销毁实体
                std::cout << "[DeathSystem] Player died! Immortal, skipping destruction.\n";
                // 移除 DeathTag（玩家不应该有这个）
                deathTags.remove(entity);
                continue;  // 跳过后续销毁逻辑
            }
            
            // ← 小怪正常销毁流程
            // 【核心修复】DeathSystem 只切换状态，不直接销毁实体！
            // 让 CleanupSystem 统一执行全组件清理
            std::cout << "[DeathSystem] Entity " << entity << " marked for cleanup.\n";
        }
    }
};

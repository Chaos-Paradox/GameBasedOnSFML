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

/**
 * @brief 死亡处理系统
 * 
 * 职责：
 * - 扫描所有 DeathTag
 * - 切换实体为 Dead 状态
 * - 【关键修复】彻底清理所有组件，防止幽灵组件残留
 * - 销毁实体 ID
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
        float dt)
    {
        (void)dt;
        
        auto entities = deathTags.entityList();
        for (Entity entity : entities) {
            // 切换为死亡状态
            if (states.has(entity)) {
                auto& state = states.get(entity);
                state.currentState = CharacterState::Dead;
                state.previousState = CharacterState::Dead;
                state.stateTimer = 0.0f;
            }
            
            // ← 【关键修复】彻底清理所有组件，防止幽灵组件残留
            if (transforms.has(entity)) transforms.remove(entity);
            if (characters.has(entity)) characters.remove(entity);
            if (hurtboxes.has(entity)) hurtboxes.remove(entity);
            if (lootDrops.has(entity)) lootDrops.remove(entity);
            if (inputs.has(entity)) inputs.remove(entity);
            
            // 销毁 DeathTag
            deathTags.remove(entity);
            
            // ← 【关键修复】最后销毁实体 ID
            ecs.destroy(entity);
        }
    }
};

#pragma once
#include "../core/Component.h"
#include "../core/ECS.h"
#include "../components/StateMachine.h"
#include "../components/DeathTag.h"

/**
 * @brief 死亡处理系统
 * 
 * 职责：
 * - 扫描所有 DeathTag
 * - 切换实体为 Dead 状态
 * - 【修复】真正销毁实体（包括所有组件）
 * - 销毁 DeathTag
 */
class DeathSystem {
public:
    void update(
        ComponentStore<StateMachineComponent>& states,
        ComponentStore<DeathTag>& deathTags,
        ECS& ecs,  // ← 新增：ECS 对象，用于真正抹杀实体
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
            
            // ← 【修复】真正从内存中抹除实体（包括 Transform, Hurtbox 等所有组件）
            ecs.destroy(entity);
            
            // 销毁 DeathTag
            deathTags.remove(entity);
        }
    }
};

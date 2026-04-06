#pragma once
#include "../core/Component.h"
#include "../components/StateMachine.h"
#include "../components/Transform.h"
#include "../components/Character.h"
#include "../components/InputCommand.h"
#include <cmath>

/**
 * @brief 位移系统（速度计算）
 * 
 * ⚠️ 关键设计：Dash 状态下绝对不修改速度！
 */
class LocomotionSystem {
public:
    void update(
        const ComponentStore<StateMachineComponent>& states,
        ComponentStore<TransformComponent>& transforms,
        const ComponentStore<CharacterComponent>& characters,
        const ComponentStore<InputCommand>& inputs,
        float dt)
    {
        (void)dt;
        
        auto entities = states.entityList();
        for (Entity entity : entities) {
            const auto& state = states.get(entity);
            
            // 安全检查
            if (!transforms.has(entity)) {
                continue;
            }
            
            // ← 【关键】状态拦截：Dash/Attack/Hurt/Dead 状态下绝对不修改速度！
            if (state.currentState == CharacterState::Dash ||
                state.currentState == CharacterState::Attack ||  // ← 新增：攻击时定住脚步
                state.currentState == CharacterState::Hurt ||
                state.currentState == CharacterState::Dead) {
                continue;  // 让 DashSystem/MovementSystem 控制速度
            }
            
            auto& transform = transforms.get(entity);
            
            if (!characters.has(entity)) {
                continue;
            }
            
            const auto& character = characters.get(entity);
            
            // 安全检查
            if (!inputs.has(entity)) {
                continue;
            }
            
            const auto& input = inputs.get(entity);
            
            // 只在 Move/Idle 状态下处理移动
            Vec2 dir = input.moveDir;
            
            // 如果有移动输入
            if (dir.x != 0.0f || dir.y != 0.0f) {
                // 保存未归一化的方向作为 facing（用于攻击方向）
                transform.facingX = dir.x;
                transform.facingY = dir.y;
                
                // 向量归一化，防止斜向移动变快
                float length = std::sqrt(dir.x * dir.x + dir.y * dir.y);
                if (length > 0.0f) {
                    dir.x /= length;
                    dir.y /= length;
                }
                
                // 应用基础速度（使用归一化后的方向）
                transform.velocity.x = dir.x * character.baseMoveSpeed;
                transform.velocity.y = dir.y * character.baseMoveSpeed;
            } else {
                transform.velocity = {0.0f, 0.0f};
            }
        }
    }
};

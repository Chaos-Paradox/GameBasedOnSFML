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
 * 职责：
 * - 读取 StateMachineComponent.currentState
 * - 读取 InputCommand.moveDir（连续向量）
 * - 读取 CharacterComponent.baseMoveSpeed
 * - 计算并写入 TransformComponent.velocity
 * 
 * ⚠️ 架构升级：从"离散指令"转向"连续向量"
 * ⚠️ 关键修复：向量归一化（Normalize），防止斜向移动变快 41%
 * 
 * ⚠️ facing 方向修复：
 * - facing 应该保存**未归一化的方向**（如 (1, -1) 表示右上）
 * - 这样攻击时 Hitbox 会出现在正确的方向
 * - 速度计算时才使用归一化后的方向
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
            
            // 只在 Move 状态下处理移动
            if (state.currentState == CharacterState::Move) {
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
            } else {
                // 非 Move 状态，速度归零
                transform.velocity = {0.0f, 0.0f};
                // facing 保持不变，保持最后移动方向
            }
        }
    }
};

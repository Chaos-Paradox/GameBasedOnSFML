#pragma once
#include "../core/Component.h"
#include "../components/Transform.h"
#include "../components/ItemData.h"
#include "../components/StateMachine.h"
#include "../components/BombComponent.h"
#include "../components/ZTransformComponent.h"
#include <cmath>

/**
 * @brief 位移执行系统（将速度化为实质距离）
 * 
 * ⚠️ 核心改动：引入智能全局摩擦力
 * - 主动发力状态（Move/Dash/Attack）免除摩擦
 * - 空中炸弹免除摩擦
 * - Idle 状态/落地炸弹/掉落物施加急停摩擦
 */
class MovementSystem {
public:
    void update(
        ComponentStore<TransformComponent>& transforms,
        ComponentStore<ItemDataComponent>& itemDatas,
        const ComponentStore<StateMachineComponent>& states,      // ← 新增：判断状态
        const ComponentStore<BombComponent>& bombs,               // ← 新增：识别炸弹
        const ComponentStore<ZTransformComponent>& zTransforms,   // ← 新增：判断高度
        float dt)
    {
        auto entities = transforms.entityList();
        for (Entity entity : entities) {
            auto& transform = transforms.get(entity);
            
            // ========== 智能摩擦力判断 ==========
            bool shouldApplyFriction = true;
            
            // 1. 如果实体有状态机，且正在主动发力（Move / Dash / Attack），免除自然摩擦
            if (states.has(entity)) {
                auto currentState = states.get(entity).currentState;
                if (currentState == CharacterState::Move || 
                    currentState == CharacterState::Dash || 
                    currentState == CharacterState::Attack) {
                    shouldApplyFriction = false;
                }
            }
            
            // 2. 专门放过正在天上飞的炸弹 (防止空中减速太快)
            if (bombs.has(entity) && zTransforms.has(entity)) {
                if (zTransforms.get(entity).z > 5.0f) {
                    shouldApplyFriction = false;
                }
            }
            
            // 3. 对需要停下的实体施加急停摩擦力
            if (shouldApplyFriction) {
                // std::pow 保证帧率独立，0.001f 代表 1 秒内速度衰减到 0.1% (极强的抓地力)
                float friction = std::pow(0.001f, dt);
                transform.velocity.x *= friction;
                transform.velocity.y *= friction;
                
                // 彻底停稳，消除微小浮点数滑动
                if (std::abs(transform.velocity.x) < 5.0f) transform.velocity.x = 0.0f;
                if (std::abs(transform.velocity.y) < 5.0f) transform.velocity.y = 0.0f;
            }
            
            // ========== 应用位移 ==========
            transform.position.x += transform.velocity.x * dt;
            transform.position.y += transform.velocity.y * dt;
        }
    }
};

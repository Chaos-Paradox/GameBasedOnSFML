#pragma once
#include "../core/Component.h"
#include "../core/Entity.h"
#include "../core/ECS.h"
#include "../components/StateMachine.h"
#include "../components/Transform.h"
#include "../components/Character.h"
#include "../components/Hitbox.h"
#include "../components/Lifetime.h"
#include "../components/AttackState.h"

/**
 * @brief 攻击判定系统
 * 
 * 职责：
 * - 监听同时拥有 StateMachineComponent 和 AttackStateComponent 的实体
 * - **只在 hitActivated == false 时创建一次 Hitbox**
 * - Hitbox 位置 = 攻击者位置 + facing 方向 * 50 像素
 * 
 * ⚠️ 修复：渲染和判定不一致
 * - AttackSystem: transform.position + bounds (bounds 包含 facing 偏移)
 * - CollisionSystem: transform.position + bounds ✓
 * - renderHitboxes: transform.position + bounds ✓
 * 三者统一！
 */
class AttackSystem {
public:
    void update(
        const ComponentStore<StateMachineComponent>& states,
        ComponentStore<AttackStateComponent>& attackStates,
        const ComponentStore<TransformComponent>& transforms,
        const ComponentStore<CharacterComponent>& characters,
        ECS& ecs,
        ComponentStore<TransformComponent>& hitboxTransforms,
        ComponentStore<HitboxComponent>& hitboxes,
        ComponentStore<LifetimeComponent>& lifetimes,
        float dt)
    {
        (void)dt;
        (void)characters;
        (void)states;
        
        // 遍历同时拥有 AttackStateComponent 的实体
        auto entities = attackStates.entityList();
        for (Entity entity : entities) {
            auto& attackState = attackStates.get(entity);
            
            // 只有 hitActivated == false 时才创建 Hitbox
            if (attackState.hitActivated) {
                continue;  // 已激活过，跳过
            }
            
            // 安全检查：确保攻击者有 Transform
            if (!transforms.has(entity)) {
                continue;
            }
            
            const auto& transform = transforms.get(entity);
            
            // 创建 Hitbox 临时实体（只执行一次）
            Entity hitboxEntity = ecs.create();
            
            // ← 修复关键：transform.position 就是角色位置
            // bounds 包含 facing 方向的偏移
            hitboxTransforms.add(hitboxEntity, {
                .position = transform.position,  // ← 使用角色位置
                .scale = {1.0f, 1.0f},
                .rotation = 0.0f,
                .velocity = {0.0f, 0.0f}
            });
            
            // ← 修复关键：bounds 包含 facing 偏移和大小
            // ← facing 现在是未归一化的方向（如 (1, -1) 表示右上）
            float offsetX = transform.facingX * 50.0f;
            float offsetY = transform.facingY * 50.0f;
            
            // ← 居中补偿：减去宽高的一半，让 bounds 的 (x,y) 表示中心点偏移
            // 这样 CollisionSystem 和 renderHitboxes 计算时：
            // center = position + bounds + size/2
            //        = position + (offsetX-20, offsetY-20) + (20, 20)
            //        = position + (offsetX, offsetY)  ← 完美！
            hitboxes.add(hitboxEntity, {
                .bounds = {offsetX - 20.0f, offsetY - 20.0f, 40, 40},  // ← 居中补偿
                .damageMultiplier = 10,
                .element = ElementType::Physical,
                .knockbackForce = 100.0f,
                .sourceEntity = entity,
                .hitHistory = {},
                .hitCount = 0,
                .active = true
            });
            
            // ← 调试输出：攻击信息
            if (entity == 1) {  // 只输出玩家的攻击
                float centerX = transform.position.x + offsetX;
                float centerY = transform.position.y + offsetY;
                std::cout << "[Attack] Player facing=(" << transform.facingX << ", " << transform.facingY 
                          << ") offset=(" << offsetX << ", " << offsetY 
                          << ") center=(" << centerX << ", " << centerY << ")\n";
            }
            
            lifetimes.add(hitboxEntity, {
                .timeLeft = 0.3f,
                .autoDestroy = true
            });
            
            // 设置 hitActivated = true，防止重复创建
            attackState.hitActivated = true;
        }
    }
};

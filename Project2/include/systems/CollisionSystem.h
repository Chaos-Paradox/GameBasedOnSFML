#pragma once
#include "../core/Component.h"
#include "../core/ECS.h"
#include "../components/Hitbox.h"
#include "../components/Hurtbox.h"
#include "../components/Transform.h"
#include "../components/ZTransformComponent.h"
#include "../components/DamageEventComponent.h"
#include "../math/Rect.h"
#include <cstdlib>
#include <iostream>

/**
 * @brief 碰撞检测系统（掷骰子裁判）
 * 
 * 职责：
 * - 遍历所有 Hitbox 和 Hurtbox
 * - AABB 碰撞检测
 * - 自伤检测（不能伤害自己）
 * - 命中历史检测（防止重复伤害）
 * - 计算伤害浮动（0.8f ~ 1.2f）
 * - 创建 DamageEventComponent 事件实体
 * 
 * ⚠️ 关键设计：不再直接挂载 DamageTag，而是创建事件实体
 * 
 * @see DamageEventComponent - 伤害事件载荷
 * @see DamageSystem - 伤害结算系统
 */
class CollisionSystem {
public:
    void update(
        const ComponentStore<HitboxComponent>& hitboxes,
        const ComponentStore<HurtboxComponent>& hurtboxes,
        const ComponentStore<TransformComponent>& hitboxTransforms,
        const ComponentStore<TransformComponent>& targetTransforms,
        const ComponentStore<ZTransformComponent>& zTransforms,  // ← 新增：Z 轴组件
        ComponentStore<DamageEventComponent>& damageEvents,  // ← 改为事件实体存储
        ECS& ecs,  // ← 新增：用于创建事件实体
        float dt)
    {
        (void)dt;
        
        auto hitboxEntities = hitboxes.entityList();
        auto hurtboxEntities = hurtboxes.entityList();
        
        for (Entity hitboxEntity : hitboxEntities) {
            // 安全检查
            if (!hitboxes.has(hitboxEntity) || !hitboxTransforms.has(hitboxEntity)) {
                continue;
            }
            
            const auto& hitbox = hitboxes.get(hitboxEntity);
            if (!hitbox.active) continue;
            
            const auto& hitboxTransform = hitboxTransforms.get(hitboxEntity);
            
            // 获取 Hitbox 的 Z 轴数据（如果没有则 z=0, height=0）
            float hitboxZ = 0.0f;
            float hitboxHeight = 0.0f;
            if (zTransforms.has(hitboxEntity)) {
                const auto& zComp = zTransforms.get(hitboxEntity);
                hitboxZ = zComp.z;
                hitboxHeight = zComp.height;
            }
            
            // 计算 Hitbox 的世界坐标（XY 平面）
            Rect hitboxWorld = {
                hitboxTransform.position.x + hitbox.bounds.x,
                hitboxTransform.position.y + hitbox.bounds.y,
                hitbox.bounds.width,
                hitbox.bounds.height
            };
            
            for (Entity hurtboxEntity : hurtboxEntities) {
                // 不能伤害自己
                if (hitboxEntity == hurtboxEntity) continue;
                if (hitbox.sourceEntity == hurtboxEntity) continue;
                
                // 安全检查
                if (!hurtboxes.has(hurtboxEntity) || !targetTransforms.has(hurtboxEntity)) {
                    continue;
                }
                
                const auto& hurtbox = hurtboxes.get(hurtboxEntity);
                
                if (hitbox.sourceEntity == INVALID_ENTITY) {
                    continue;
                }
                
                // 命中历史检测
                if (hasHitEntity(hitbox, hurtboxEntity)) continue;
                
                const auto& targetTransform = targetTransforms.get(hurtboxEntity);
                
                // 获取 Hurtbox 的 Z 轴数据（如果没有则 z=0, height=0）
                float hurtboxZ = 0.0f;
                float hurtboxHeight = 0.0f;
                if (zTransforms.has(hurtboxEntity)) {
                    const auto& zComp = zTransforms.get(hurtboxEntity);
                    hurtboxZ = zComp.z;
                    hurtboxHeight = zComp.height;
                }
                
                // 计算 Hurtbox 的世界坐标（XY 平面）
                Rect hurtboxWorld = {
                    targetTransform.position.x + hurtbox.bounds.x,
                    targetTransform.position.y + hurtbox.bounds.y,
                    hurtbox.bounds.width,
                    hurtbox.bounds.height
                };
                
                // AABB 碰撞检测（XY 平面）
                if (hitboxWorld.overlaps(hurtboxWorld)) {
                    // ========== 【核心改动】Z 轴豁免判定 ==========
                    // 影子重叠了！这时候再看 Z 轴高度
                    float hitboxBottom = hitboxZ;
                    float hitboxTop = hitboxZ + hitboxHeight;
                    float hurtboxBottom = hurtboxZ;
                    float hurtboxTop = hurtboxZ + hurtboxHeight;
                    
                    // 如果 Z 轴没有交集，说明一个在空中一个在地上，豁免碰撞
                    // 逻辑：!(aBottom > bTop || aTop < bBottom) = Z 轴有交集
                    bool zAxisOverlap = !(hitboxBottom > hurtboxTop || hitboxTop < hurtboxBottom);
                    
                    if (!zAxisOverlap) {
                        // Z 轴豁免：虽然 XY 重叠，但高度不同，不会受伤
                        std::cout << "[Collision] Z-axis豁免：hitboxZ=" << hitboxZ 
                                  << " hurtboxZ=" << hurtboxZ << "\n";
                        continue;
                    }
                    // ========== Z 轴判定结束 ==========
                    
                    // 添加到命中历史
                    HitboxComponent& mutableHitbox = const_cast<HitboxComponent&>(hitbox);
                    addToHitHistory(mutableHitbox, hurtboxEntity);
                    
                    // 计算伤害浮动并创建事件实体
                    Entity eventEntity = createDamageEvent(
                        ecs, damageEvents,
                        hitbox, hurtboxEntity, hitbox.sourceEntity,
                        hitboxWorld, hurtboxWorld
                    );
                    
                    (void)eventEntity;  // 事件实体已创建，由 DamageSystem 处理
                }
            }
        }
    }
    
private:
    /**
     * @brief 创建伤害事件实体
     * 
     * @param ecs ECS 实例（用于创建实体）
     * @param damageEvents DamageEventComponent 存储
     * @param hitbox 命中的 Hitbox
     * @param target 受击者实体 ID
     * @param attacker 攻击者实体 ID
     * @param hitboxWorld Hitbox 世界坐标
     * @param hurtboxWorld Hurtbox 世界坐标
     * @return 事件实体 ID
     */
    Entity createDamageEvent(
        ECS& ecs,
        ComponentStore<DamageEventComponent>& damageEvents,
        const HitboxComponent& hitbox,
        Entity target,
        Entity attacker,
        const Rect& hitboxWorld,
        const Rect& hurtboxWorld)
    {
        Entity eventEntity = ecs.create();
        
        // ← 【核心改动】计算伤害浮动（0.8f ~ 1.2f）
        float randomMultiplier = 0.8f + (static_cast<float>(std::rand()) / RAND_MAX) * 0.4f;
        
        // ← 【核心改动】判定暴击（浮动倍率 > 1.1f）
        bool isCritical = (randomMultiplier > 1.1f);
        
        // ← 【核心改动】计算最终伤害
        int actualDamage = static_cast<int>(hitbox.damageMultiplier * randomMultiplier);
        
        // ← 【核心改动】计算打击位置（两个碰撞框的中心点）
        float hitX = (hitboxWorld.x + hitboxWorld.width / 2.0f + 
                      hurtboxWorld.x + hurtboxWorld.width / 2.0f) / 2.0f;
        float hitY = (hitboxWorld.y + hitboxWorld.height / 2.0f + 
                      hurtboxWorld.y + hurtboxWorld.height / 2.0f) / 2.0f;
        
        // 挂载 DamageEventComponent
        damageEvents.add(eventEntity, {
            .target = target,
            .actualDamage = actualDamage,
            .hitPosition = {hitX, hitY},
            .isCritical = isCritical,
            .attacker = attacker,
            .timestamp = 0.0f  // 由主循环设置
        });
        
        std::cout << "[Collision] Created damage event: target=" << target 
                  << " damage=" << actualDamage 
                  << " (multiplier=" << randomMultiplier 
                  << ", crit=" << (isCritical ? "YES" : "NO") << ")\n";
        
        return eventEntity;
    }
    
    bool hasHitEntity(const HitboxComponent& hitbox, Entity target) const {
        for (int i = 0; i < hitbox.hitCount && i < HitboxComponent::MAX_HIT_COUNT; ++i) {
            if (hitbox.hitHistory[i] == target) {
                return true;
            }
        }
        return false;
    }
    
    void addToHitHistory(HitboxComponent& hitbox, Entity target) {
        if (hitbox.hitCount < HitboxComponent::MAX_HIT_COUNT) {
            hitbox.hitHistory[hitbox.hitCount++] = target;
        }
    }
};

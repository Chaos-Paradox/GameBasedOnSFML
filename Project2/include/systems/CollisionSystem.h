#pragma once
#include "../core/Component.h"
#include "../core/ECS.h"
#include "../components/Hitbox.h"
#include "../components/Hurtbox.h"
#include "../components/Transform.h"
#include "../components/ZTransformComponent.h"
#include "../components/DamageEventComponent.h"
#include <cstdlib>
#include <iostream>

/**
 * @brief 碰撞检测系统（2.5D 圆柱体判定 - 圆形）
 * 
 * ⚠️ 架构设计：
 * - XY 平面：圆形相交检测（distance <= radiusA + radiusB）
 * - Z 轴高度：圆柱体相交检测（bottom/top 重叠）
 * - 只有 XY && Z 同时相交才触发伤害
 * 
 * 核心算法：
 * 1. 计算 Hitbox 世界坐标 = attacker.position + hitbox.offset
 * 2. 计算 Hurtbox 世界坐标 = victim.position + hurtbox.offset
 * 3. 圆形相交：distance <= (hitbox.radius + hurtbox.radius)
 * 4. Z 轴相交：!(bottomA > topB || topA < bottomB)
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
        const ComponentStore<ZTransformComponent>& zTransforms,
        ComponentStore<DamageEventComponent>& damageEvents,
        ECS& ecs,
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
            
            // ========== 1. 获取 Hitbox 的 Z 轴高度数据 ==========
            // 如果实体没有 ZTransformComponent，默认 z=0, height=40.0f（默认人高）
            float hitboxZ = 0.0f;
            float hitboxHeight = 40.0f;  // 默认人高
            if (zTransforms.has(hitboxEntity)) {
                const auto& zComp = zTransforms.get(hitboxEntity);
                hitboxZ = zComp.z;
                hitboxHeight = zComp.height;
            }
            
            // 获取攻击者实体的 Z 轴数据（Hitbox 的发射源）
            float attackerZ = 0.0f;
            float attackerHeight = 40.0f;
            if (hitbox.sourceEntity != INVALID_ENTITY && zTransforms.has(hitbox.sourceEntity)) {
                const auto& zComp = zTransforms.get(hitbox.sourceEntity);
                attackerZ = zComp.z;
                attackerHeight = zComp.height;
            }
            
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
                
                // ========== 2. 获取 Hurtbox 的 Z 轴高度数据 ==========
                // 如果实体没有 ZTransformComponent，默认 z=0, height=40.0f（默认人高）
                float victimZ = 0.0f;
                float victimHeight = 40.0f;  // 默认人高
                if (zTransforms.has(hurtboxEntity)) {
                    const auto& zComp = zTransforms.get(hurtboxEntity);
                    victimZ = zComp.z;
                    victimHeight = zComp.height;
                }
                
                // ========== 3. 条件 A：XY 平面圆形相交检测 ==========
                // 计算 Hitbox 世界坐标 = attacker.position + hitbox.offset
                float hitboxWorldX = hitboxTransform.position.x + hitbox.offset.x;
                float hitboxWorldY = hitboxTransform.position.y + hitbox.offset.y;
                
                // 计算 Hurtbox 世界坐标 = victim.position + hurtbox.offset
                float hurtboxWorldX = targetTransform.position.x + hurtbox.offset.x;
                float hurtboxWorldY = targetTransform.position.y + hurtbox.offset.y;
                
                // 计算圆心距离
                float dx = hitboxWorldX - hurtboxWorldX;
                float dy = hitboxWorldY - hurtboxWorldY;
                float distance = std::sqrt(dx * dx + dy * dy);
                
                // 圆形相交：distance <= (hitbox.radius + hurtbox.radius)
                float minDistance = hitbox.radius + hurtbox.radius;
                bool xyIntersect = distance <= minDistance;
                
                // XY 平面不相交，直接跳过
                if (!xyIntersect) {
                    continue;
                }
                
                // ========== 4. 条件 B：Z 轴高度相交检测 ==========
                // 使用攻击者（sourceEntity）的 Z 轴数据，而不是 Hitbox 实体的
                float attackerBottom = attackerZ;
                float attackerTop = attackerZ + attackerHeight;
                
                float victimBottom = victimZ;
                float victimTop = victimZ + victimHeight;
                
                // Z 轴重叠判定：A 的底部低于 B 的顶部，且 A 的顶部高于 B 的底部
                // 逻辑：!(attackerBottom > victimTop || attackerTop < victimBottom)
                bool zIntersect = !(attackerBottom > victimTop || attackerTop < victimBottom);
                
                // ========== 5. 最终判定：XY && Z 同时相交 ==========
                if (!zIntersect) {
                    // Z 轴豁免：虽然 XY 重叠，但高度不同（例如跳跃躲避）
                    std::cout << "[Collision] Z-axis 豁免！attackerZ=" << attackerZ 
                              << " victimZ=" << victimZ 
                              << " (attackerTop=" << attackerTop 
                              << ", victimBottom=" << victimBottom << ")\n";
                    continue;
                }
                
                // ========== 6. 碰撞成立！创建伤害事件 ==========
                std::cout << "[Collision] ✓ 命中！XY 圆形相交 distance=" << distance 
                          << " (min=" << minDistance << ")"
                          << " Z 轴相交 attackerZ=" << attackerZ 
                          << " victimZ=" << victimZ << "\n";
                
                // 添加到命中历史
                HitboxComponent& mutableHitbox = const_cast<HitboxComponent&>(hitbox);
                addToHitHistory(mutableHitbox, hurtboxEntity);
                
                // 计算打击位置（两个圆心的中点）
                float hitX = (hitboxWorldX + hurtboxWorldX) / 2.0f;
                float hitY = (hitboxWorldY + hurtboxWorldY) / 2.0f;
                
                // 创建伤害事件实体
                Entity eventEntity = createDamageEvent(
                    ecs, damageEvents,
                    hitbox, hurtboxEntity, hitbox.sourceEntity,
                    hitX, hitY
                );
                
                (void)eventEntity;  // 事件实体已创建，由 DamageSystem 处理
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
     * @param hitX 打击位置 X
     * @param hitY 打击位置 Y
     * @return 事件实体 ID
     */
    Entity createDamageEvent(
        ECS& ecs,
        ComponentStore<DamageEventComponent>& damageEvents,
        const HitboxComponent& hitbox,
        Entity target,
        Entity attacker,
        float hitX, float hitY)
    {
        Entity eventEntity = ecs.create();
        
        // 计算伤害浮动（0.8f ~ 1.2f）
        float randomMultiplier = 0.8f + (static_cast<float>(std::rand()) / RAND_MAX) * 0.4f;
        
        // 判定暴击（浮动倍率 > 1.1f）
        bool isCritical = (randomMultiplier > 1.1f);
        
        // 计算最终伤害
        int actualDamage = static_cast<int>(hitbox.damageMultiplier * randomMultiplier);
        
        // 挂载 DamageEventComponent
        damageEvents.add(eventEntity, {
            .target = target,
            .actualDamage = actualDamage,
            .hitPosition = {hitX, hitY},
            .isCritical = isCritical,
            .attacker = attacker,
            .timestamp = 0.0f  // 由主循环设置
        });
        
        std::cout << "[Collision] 伤害事件：target=" << target 
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

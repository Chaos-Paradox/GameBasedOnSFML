#pragma once
#include "../core/Component.h"
#include "../components/Hitbox.h"
#include "../components/Hurtbox.h"
#include "../components/Transform.h"
#include "../components/DamageTag.h"
#include "../math/Rect.h"
#include <iostream>

/**
 * @brief 碰撞检测系统
 * 
 * 职责：
 * - 遍历所有 Hitbox 和 Hurtbox
 * - AABB 碰撞检测
 * - 自伤检测（不能伤害自己）
 * - 命中历史检测（防止重复伤害）
 * - 挂载 DamageTag 到目标
 * 
 * ⚠️ Bug 2 修复：正确应用 Hitbox 的 bounds 到世界坐标
 * 
 * TODO:
 * - [ ] 添加空间分区优化（四叉树）
 * - [ ] 添加阵营/友军检测
 * - [ ] 添加击退方向计算
 * - [ ] 添加碰撞音效
 * - [ ] 添加命中特效
 * - [ ] 添加命中停顿（Hitstop）
 */
class CollisionSystem {
public:
    void update(
        const ComponentStore<HitboxComponent>& hitboxes,
        const ComponentStore<HurtboxComponent>& hurtboxes,
        const ComponentStore<TransformComponent>& hitboxTransforms,
        const ComponentStore<TransformComponent>& targetTransforms,
        ComponentStore<DamageTag>& damageTags,
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
            
            // ← Bug 2 修复：正确计算 Hitbox 的世界坐标
            // hitbox.bounds 是相对于 hitboxTransform.position 的偏移
            // 所以世界坐标 = transform.position + bounds 偏移
            Rect hitboxWorld = {
                hitboxTransform.position.x + hitbox.bounds.x,  // 左边界
                hitboxTransform.position.y + hitbox.bounds.y,  // 上边界
                hitbox.bounds.width,                            // 宽度
                hitbox.bounds.height                            // 高度
            };
            
            // ← 调试输出已禁用（高频日志）
            // float hitboxCenterX = hitboxWorld.x + hitboxWorld.width / 2.0f;
            // float hitboxCenterY = hitboxWorld.y + hitboxWorld.height / 2.0f;
            // std::cout << "[Collision] Hitbox center=(" << hitboxCenterX << ", " << hitboxCenterY 
            //           << ") size=" << hitboxWorld.width << "x" << hitboxWorld.height << "\n";
            
            for (Entity hurtboxEntity : hurtboxEntities) {
                // ← Bug 2 修复：不能伤害自己
                if (hitboxEntity == hurtboxEntity) continue;
                
                // ← Bug 2 修复：检查 Hitbox 来源是否等于 Hurtbox 实体
                if (hitbox.sourceEntity == hurtboxEntity) {
                    continue;  // 不能伤害自己
                }
                
                // 安全检查
                if (!hurtboxes.has(hurtboxEntity) || !targetTransforms.has(hurtboxEntity)) {
                    continue;
                }
                
                const auto& hurtbox = hurtboxes.get(hurtboxEntity);
                
                // 安全检查
                if (hitbox.sourceEntity == INVALID_ENTITY) {
                    continue;
                }
                
                // 命中历史检测
                if (hasHitEntity(hitbox, hurtboxEntity)) continue;
                
                // ← Bug 2 修复：从 targetTransforms 获取 Hurtbox 位置
                const auto& targetTransform = targetTransforms.get(hurtboxEntity);
                
                // 正确计算 Hurtbox 的世界坐标
                Rect hurtboxWorld = {
                    targetTransform.position.x + hurtbox.bounds.x,
                    targetTransform.position.y + hurtbox.bounds.y,
                    hurtbox.bounds.width,
                    hurtbox.bounds.height
                };
                
                // ← 调试输出已禁用（高频日志）
                // float hurtboxCenterX = hurtboxWorld.x + hurtboxWorld.width / 2.0f;
                // float hurtboxCenterY = hurtboxWorld.y + hurtboxWorld.height / 2.0f;
                // std::cout << "[Collision] Hurtbox center=(" << hurtboxCenterX << ", " << hurtboxCenterY 
                //           << ") size=" << hurtboxWorld.width << "x" << hurtboxWorld.height;
                
                // AABB 碰撞检测
                if (hitboxWorld.overlaps(hurtboxWorld)) {
                    // std::cout << " -> HIT!\n";  // ← 已禁用
                    
                    // 添加到命中历史
                    HitboxComponent& mutableHitbox = const_cast<HitboxComponent&>(hitbox);
                    addToHitHistory(mutableHitbox, hurtboxEntity);
                    
                    // 挂载 DamageTag
                    damageTags.add(hurtboxEntity, {
                        .damage = static_cast<float>(hitbox.damageMultiplier)
                    });
                    
                    std::cout << "[DamageSystem] Entity " << hurtboxEntity 
                              << " took " << hitbox.damageMultiplier << " damage from Entity "
                              << hitbox.sourceEntity << "\n";
                } else {
                    std::cout << " -> MISS\n";
                }
            }
        }
    }
    
private:
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

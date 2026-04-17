#pragma once
#include "core/GameWorld.h"
#include <cmath>
#include <iostream>
#include <unordered_set>

/**
 * @brief 碰撞检测系统（2.5D 圆柱体判定 - 圆形）
 *
 * ⚠️ 重构（ECS 纯净原则）：
 * - 不再通过 world.damageEvents 创建 ECS 实体，改为写入 world.events 事件队列
 * - hitTargets 由系统内部维护（每帧创建新的 set，不从组件读取/写入）
 */
class CollisionSystem {
public:
    void update(GameWorld& world, float dt)
    {
        (void)dt;

        // 每帧重置打击记录（事件不跨帧）
        frameHitTargets.clear();

        auto hitboxEntities = world.hitboxes.entityList();
        auto hurtboxEntities = world.hurtboxes.entityList();

        for (Entity hitboxEntity : hitboxEntities) {
            if (!world.hitboxes.has(hitboxEntity) || !world.transforms.has(hitboxEntity)) continue;

            const auto& hitbox = world.hitboxes.get(hitboxEntity);
            if (!hitbox.active) continue;

            const auto& hitboxTransform = world.transforms.get(hitboxEntity);

            float hitboxZ = 0.0f;
            float hitboxHeight = 40.0f;
            if (world.zTransforms.has(hitboxEntity)) {
                const auto& zComp = world.zTransforms.get(hitboxEntity);
                hitboxZ = zComp.z;
                hitboxHeight = zComp.height;
            }

            float attackerZ = 0.0f;
            float attackerHeight = 40.0f;
            if (hitbox.sourceEntity != INVALID_ENTITY && world.zTransforms.has(hitbox.sourceEntity)) {
                const auto& zComp = world.zTransforms.get(hitbox.sourceEntity);
                attackerZ = zComp.z;
                attackerHeight = zComp.height;
            }

            for (Entity hurtboxEntity : hurtboxEntities) {
                if (hitboxEntity == hurtboxEntity) continue;
                if (hitbox.sourceEntity == hurtboxEntity) continue;
                if (!world.hurtboxes.has(hurtboxEntity) || !world.transforms.has(hurtboxEntity)) continue;

                const auto& hurtbox = world.hurtboxes.get(hurtboxEntity);
                if (hitbox.sourceEntity == INVALID_ENTITY) continue;

                // 系统内部记录防止多帧重复伤害
                uint64_t hitKey = (static_cast<uint64_t>(hitboxEntity) << 32) | static_cast<uint64_t>(hurtboxEntity);
                if (frameHitTargets.count(hitKey)) {
                    std::cout << "[Collision] ⚠️ 跳过重复命中！Hitbox=" << hitboxEntity << " target=" << hurtboxEntity << "\n";
                    continue;
                }

                const auto& targetTransform = world.transforms.get(hurtboxEntity);

                float victimZ = 0.0f;
                float victimHeight = 40.0f;
                if (world.zTransforms.has(hurtboxEntity)) {
                    const auto& zComp = world.zTransforms.get(hurtboxEntity);
                    victimZ = zComp.z;
                    victimHeight = zComp.height;
                }

                float hitboxWorldX = hitboxTransform.position.x + hitbox.offset.x;
                float hitboxWorldY = hitboxTransform.position.y + hitbox.offset.y;
                float hurtboxWorldX = targetTransform.position.x + hurtbox.offset.x;
                float hurtboxWorldY = targetTransform.position.y + hurtbox.offset.y;

                float dx = hitboxWorldX - hurtboxWorldX;
                float dy = hitboxWorldY - hurtboxWorldY;
                float distance = std::sqrt(dx * dx + dy * dy);
                float minDistance = hitbox.radius + hurtbox.radius;
                bool xyIntersect = distance <= minDistance;

                if (!xyIntersect) continue;

                float attackerBottom = attackerZ;
                float attackerTop = attackerZ + attackerHeight;
                float victimBottom = victimZ;
                float victimTop = victimZ + victimHeight;
                bool zIntersect = !(attackerBottom > victimTop || attackerTop < victimBottom);

                if (!zIntersect) {
                    std::cout << "[Collision] Z-axis 豁免！attackerZ=" << attackerZ
                              << " victimZ=" << victimZ << "\n";
                    continue;
                }

                std::cout << "[Collision] ✓ 命中！XY 圆形相交 distance=" << distance
                          << " (min=" << minDistance << ")"
                          << " Z 轴相交 attackerZ=" << attackerZ
                          << " victimZ=" << victimZ << "\n";

                // 记录本次命中
                frameHitTargets.insert(hitKey);

                float hitX = (hitboxWorldX + hurtboxWorldX) / 2.0f;
                float hitY = (hitboxWorldY + hurtboxWorldY) / 2.0f;

                float knockbackDirX = hurtboxWorldX - hitboxWorldX;
                float knockbackDirY = hurtboxWorldY - hitboxWorldY;
                float knockbackLen = std::sqrt(knockbackDirX * knockbackDirX + knockbackDirY * knockbackDirY);
                if (knockbackLen > 0.001f) {
                    knockbackDirX /= knockbackLen;
                    knockbackDirY /= knockbackLen;
                } else {
                    knockbackDirX = 1.0f;
                    knockbackDirY = 0.0f;
                }

                // 写入事件队列（不创建 ECS 实体）
                createDamageEvent(world, hitbox, hurtboxEntity, hitbox.sourceEntity,
                    hitX, hitY, knockbackDirX, knockbackDirY,
                    hitbox.knockbackXY, hitbox.knockbackZ);
            }
        }
    }

private:
    // 系统内部状态：本帧已打击的 (hitbox, target) 对
    std::unordered_set<uint64_t> frameHitTargets;

    void createDamageEvent(
        GameWorld& world,
        const HitboxComponent& hitbox,
        Entity target, Entity attacker,
        float hitX, float hitY,
        float knockbackDirX, float knockbackDirY,
        float knockbackXY, float knockbackZ)
    {
        float randomMultiplier = 0.8f + (static_cast<float>(std::rand()) / RAND_MAX) * 0.4f;
        bool isCritical = (randomMultiplier > 1.1f);
        int actualDamage = static_cast<int>(hitbox.damageMultiplier * randomMultiplier);

        world.events.damageEvents.push_back({
            .target = target,
            .actualDamage = actualDamage,
            .hitPosition = {hitX, hitY},
            .isCritical = isCritical,
            .hitDirection = {knockbackDirX, knockbackDirY},
            .knockbackXY = knockbackXY,
            .knockbackZ = knockbackZ,
            .attacker = attacker
        });

        std::cout << "[Collision] 伤害事件：target=" << target
                  << " damage=" << actualDamage
                  << " knockbackXY=" << knockbackXY
                  << " knockbackZ=" << knockbackZ << "\n";
    }
};

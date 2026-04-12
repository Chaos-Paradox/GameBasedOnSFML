#pragma once
#include "core/GameWorld.h"
#include <cstdlib>
#include <iostream>

/**
 * @brief 碰撞检测系统（2.5D 圆柱体判定 - 圆形）
 */
class CollisionSystem {
public:
    void update(GameWorld& world, float dt)
    {
        (void)dt;

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

                if (hitbox.hitTargets.find(hurtboxEntity) != hitbox.hitTargets.end()) {
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

                HitboxComponent& mutableHitbox = const_cast<HitboxComponent&>(hitbox);
                mutableHitbox.hitTargets.insert(hurtboxEntity);

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

                createDamageEvent(world, hitbox, hurtboxEntity, hitbox.sourceEntity,
                    hitX, hitY, knockbackDirX, knockbackDirY,
                    hitbox.knockbackXY, hitbox.knockbackZ);
            }
        }
    }

private:
    Entity createDamageEvent(
        GameWorld& world,
        const HitboxComponent& hitbox,
        Entity target, Entity attacker,
        float hitX, float hitY,
        float knockbackDirX, float knockbackDirY,
        float knockbackXY, float knockbackZ)
    {
        Entity eventEntity = world.ecs.create();

        float randomMultiplier = 0.8f + (static_cast<float>(std::rand()) / RAND_MAX) * 0.4f;
        bool isCritical = (randomMultiplier > 1.1f);
        int actualDamage = static_cast<int>(hitbox.damageMultiplier * randomMultiplier);

        world.damageEvents.add(eventEntity, {
            .target = target,
            .actualDamage = actualDamage,
            .hitPosition = {hitX, hitY},
            .isCritical = isCritical,
            .hitDirection = {knockbackDirX, knockbackDirY},
            .knockbackXY = knockbackXY,
            .knockbackZ = knockbackZ,
            .attacker = attacker,
            .timestamp = 0.0f
        });

        std::cout << "[Collision] 伤害事件：target=" << target
                  << " damage=" << actualDamage
                  << " knockbackXY=" << knockbackXY
                  << " knockbackZ=" << knockbackZ << "\n";

        return eventEntity;
    }
};

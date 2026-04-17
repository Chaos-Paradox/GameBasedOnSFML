#pragma once
#include "core/GameWorld.h"
#include <vector>
#include <algorithm>
#include <iostream>

/**
 * @brief 清理系统（延迟销毁 + 孟婆汤）
 *
 * ⚠️ 重构（ECS 纯净原则）：
 * - LifetimeComponent.autoDestroy 已移除 → 所有到期 Lifetime 实体统一销毁
 *
 * 执行时机：主循环最末尾（渲染之后）
 */
class CleanupSystem {
public:
    void update(GameWorld& world, float dt)
    {
        // ========== 1. 收集所有需要销毁的实体 ==========
        std::vector<Entity> entitiesToDestroy;

        // 收集 DeathTag 实体
        auto deathEntities = world.deathTags.entityList();
        for (Entity entity : deathEntities) {
            world.deathTags.remove(entity);
            entitiesToDestroy.push_back(entity);
        }

        // 收集到期 Lifetime 实体
        auto lifetimeEntities = world.lifetimes.entityList();
        for (Entity entity : lifetimeEntities) {
            if (!world.lifetimes.has(entity)) continue;

            auto& lifetime = world.lifetimes.get(entity);
            lifetime.timeLeft -= dt;

            if (lifetime.timeLeft <= 0.0f) {
                if (std::find(entitiesToDestroy.begin(), entitiesToDestroy.end(), entity) == entitiesToDestroy.end()) {
                    entitiesToDestroy.push_back(entity);
                }
            }
        }

        // ========== 2. 统一执行抹除与销毁 ==========
        for (Entity entity : entitiesToDestroy) {
            world.states.remove(entity);
            world.transforms.remove(entity);
            world.characters.remove(entity);
            world.inputs.remove(entity);
            world.hurtboxes.remove(entity);
            world.hitboxes.remove(entity);
            world.attackStates.remove(entity);
            world.lifetimes.remove(entity);
            world.damageTags.remove(entity);
            world.deathTags.remove(entity);
            world.lootDrops.remove(entity);
            world.itemDatas.remove(entity);
            world.pickupBoxes.remove(entity);
            world.magnets.remove(entity);
            world.evolutions.remove(entity);
            world.dashes.remove(entity);
            world.bombs.remove(entity);
            world.attachedComponents.remove(entity);
            world.colliders.remove(entity);
            world.zTransforms.remove(entity);
            world.damageTexts.remove(entity);
            world.throwables.remove(entity);
            world.momentums.remove(entity);

            std::cout << "[CLEANUP] 🧹 Wiping and Destroying ID: " << (uint32_t)entity << std::endl;

            world.ecs.destroy(entity);
        }
    }
};

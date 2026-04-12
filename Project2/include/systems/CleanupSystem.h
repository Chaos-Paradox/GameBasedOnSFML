#pragma once
#include "core/GameWorld.h"
#include <vector>
#include <algorithm>

/**
 * @brief 清理系统（延迟销毁 + 孟婆汤）
 *
 * ⚠️ 架构设计：
 * - 在帧末统一清理所有带 DeathTag 的实体
 * - 清理到期的 Lifetime 实体
 * - 【核心】销毁前清空所有组件数据（防止实体复用污染）
 * - 防止 std::out_of_range 崩溃
 *
 * 执行时机：主循环最末尾（渲染之后）
 *
 * ✅ 重构后：从 26 个参数 → GameWorld& world + float dt
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

            if (lifetime.timeLeft <= 0.0f && lifetime.autoDestroy) {
                if (std::find(entitiesToDestroy.begin(), entitiesToDestroy.end(), entity) == entitiesToDestroy.end()) {
                    entitiesToDestroy.push_back(entity);
                }
            }
        }

        // ========== 2. 统一执行抹除与销毁 ==========
        for (Entity entity : entitiesToDestroy) {
            // 灌入孟婆汤：清空所有组件数据
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

            std::cout << "[CLEANUP] 🧹 Wiping and Destroying ID: " << (uint32_t)entity << std::endl;

            // 抹除所有记忆后，安全销毁 ID
            world.ecs.destroy(entity);
        }
    }
};

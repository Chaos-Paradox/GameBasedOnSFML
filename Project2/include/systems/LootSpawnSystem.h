#pragma once
#include "core/GameWorld.h"
#include <cstdlib>
#include <unordered_set>

/**
 * @brief 掉落生成系统
 *
 * ⚠️ 重构（ECS 纯净原则）：
 * - hasDropped 已从 LootDropComponent 移除 → 由系统内部已处理集合维护
 */
class LootSpawnSystem {
public:
    void update(GameWorld& world, float dt)
    {
        (void)dt;

        auto entities = world.deathTags.entityList();
        for (Entity entity : entities) {
            if (!world.lootDrops.has(entity) || !world.transforms.has(entity)) continue;

            // 系统内部防止重复掉落
            if (hasDroppedSet.count(entity)) continue;

            const auto& lootDrop = world.lootDrops.get(entity);
            const auto& transform = world.transforms.get(entity);

            for (int i = 0; i < lootDrop.lootCount; i++) {
                const auto& entry = lootDrop.lootTable[i];
                float rand = static_cast<float>(std::rand()) / RAND_MAX;

                if (rand <= entry.dropChance) {
                    Entity lootEntity = world.ecs.create();

                    float jitterX = (static_cast<float>(std::rand()) / RAND_MAX) * 40.0f - 20.0f;
                    float jitterY = (static_cast<float>(std::rand()) / RAND_MAX) * 40.0f - 20.0f;

                    world.transforms.add(lootEntity, {
                        .position = {transform.position.x + jitterX, transform.position.y + jitterY},
                        .scale = {1.0f, 1.0f},
                        .rotation = 0.0f,
                        .velocity = {
                            (static_cast<float>(std::rand()) / RAND_MAX) * 120.0f - 60.0f,
                            (static_cast<float>(std::rand()) / RAND_MAX) * -120.0f - 80.0f
                        }
                    });

                    int amount = entry.minCount;
                    if (entry.maxCount > entry.minCount) {
                        amount += std::rand() % (entry.maxCount - entry.minCount + 1);
                    }

                    world.itemDatas.add(lootEntity, {
                        .itemId = entry.itemId,
                        .amount = amount,
                        .isPickupable = true
                    });

                    world.pickupBoxes.add(lootEntity, {
                        .width = 20.0f,
                        .height = 20.0f
                    });
                }
            }

            // 标记为已处理
            hasDroppedSet.insert(entity);
        }
    }

private:
    std::unordered_set<Entity> hasDroppedSet;
};

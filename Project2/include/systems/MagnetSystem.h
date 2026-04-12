#pragma once
#include "core/GameWorld.h"
#include <cmath>

/**
 * @brief 磁力吸附系统
 */
class MagnetSystem {
public:
    void update(GameWorld& world, float dt)
    {
        auto magnetEntities = world.magnets.entityList();
        for (Entity player : magnetEntities) {
            if (!world.transforms.has(player)) continue;

            const auto& playerTransform = world.transforms.get(player);
            const auto& playerMagnet = world.magnets.get(player);

            if (playerMagnet.magnetRadius <= 0.0f) continue;

            auto lootEntities = world.itemDatas.entityList();
            for (Entity loot : lootEntities) {
                if (!world.transforms.has(loot)) continue;

                auto& lootTransform = world.transforms.get(loot);

                auto itemData = world.itemDatas.get(loot);
                if (itemData.magnetImmunityTimer > 0.0f) {
                    itemData.magnetImmunityTimer -= dt;
                    world.itemDatas.add(loot, itemData);

                    lootTransform.velocity.x *= 0.9f;
                    lootTransform.velocity.y *= 0.9f;
                    continue;
                }

                float dx = playerTransform.position.x - lootTransform.position.x;
                float dy = playerTransform.position.y - lootTransform.position.y;
                float distance = std::sqrt(dx * dx + dy * dy);

                if (distance < playerMagnet.magnetRadius && distance > 0.0f) {
                    float dirX = dx / distance;
                    float dirY = dy / distance;
                    lootTransform.velocity.x = dirX * playerMagnet.magnetSpeed;
                    lootTransform.velocity.y = dirY * playerMagnet.magnetSpeed;
                } else {
                    lootTransform.velocity.x *= 0.9f;
                    lootTransform.velocity.y *= 0.9f;
                }
            }
        }
    }
};

#pragma once
#include "core/GameWorld.h"
#include <cmath>
#include <unordered_map>

/**
 * @brief 磁力吸附系统
 *
 * ⚠️ 重构（ECS 纯净原则）：
 * - magnetImmunityTimer 已从 ItemDataComponent 移除 → 由系统内部映射维护
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

                // 磁吸免疫期（系统内部维护，默认 0.25s）
                auto it = magnetImmunities.find(loot);
                if (it != magnetImmunities.end()) {
                    it->second -= dt;
                    if (it->second <= 0.0f) {
                        magnetImmunities.erase(it);
                    } else {
                        lootTransform.velocity.x *= 0.9f;
                        lootTransform.velocity.y *= 0.9f;
                        continue;
                    }
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

private:
    std::unordered_map<Entity, float> magnetImmunities;
};

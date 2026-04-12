#pragma once
#include "core/GameWorld.h"
#include "math/Rect.h"
#include <vector>

/**
 * @brief 拾取判定系统
 */
class PickupSystem {
public:
    void update(GameWorld& world, float dt)
    {
        (void)dt;
        std::vector<Entity> entitiesToDestroy;

        auto playerEntities = world.evolutions.entityList();
        for (Entity player : playerEntities) {
            if (!world.transforms.has(player)) continue;

            const auto& pTrans = world.transforms.get(player);
            auto& evo = world.evolutions.get(player);

            Rect playerRect{
                pTrans.position.x - 20.0f,
                pTrans.position.y - 20.0f,
                40.0f,
                40.0f
            };

            auto lootEntities = world.itemDatas.entityList();
            for (Entity loot : lootEntities) {
                bool alreadyMarked = false;
                for (auto e : entitiesToDestroy) {
                    if (e == loot) { alreadyMarked = true; break; }
                }
                if (alreadyMarked) continue;

                if (!world.transforms.has(loot) || !world.pickupBoxes.has(loot)) continue;

                const auto& lTrans = world.transforms.get(loot);
                const auto& pBox = world.pickupBoxes.get(loot);
                const auto& item = world.itemDatas.get(loot);

                Rect lootRect{
                    lTrans.position.x - pBox.width / 2.0f,
                    lTrans.position.y - pBox.height / 2.0f,
                    pBox.width,
                    pBox.height
                };

                if (playerRect.overlaps(lootRect)) {
                    evo.evolutionPoints += item.amount;
                    evo.totalEarned += item.amount;
                    entitiesToDestroy.push_back(loot);
                    std::cout << "[Pickup] Collected item! Points: " << evo.evolutionPoints << std::endl;
                }
            }
        }

        for (Entity e : entitiesToDestroy) {
            world.transforms.remove(e);
            world.itemDatas.remove(e);
            world.pickupBoxes.remove(e);
            world.magnets.remove(e);
            world.ecs.destroy(e);
        }
    }
};

#pragma once
#include "../core/Component.h"
#include "../components/Transform.h"
#include "../components/Evolution.h"
#include "../components/ItemData.h"
#include "../components/PickupBox.h"
#include "../math/Rect.h"

/**
 * @brief 拾取判定系统
 * 
 * 职责：
 * - 监听玩家（Transform + Evolution）
 * - 监听掉落物（Transform + ItemData + PickupBox）
 * - 计算 AABB 碰撞
 * - 拾取成功：增加进化点数，销毁掉落物
 * 
 * ⚠️ 架构原则：单一职责
 * - 不与 CollisionSystem 混用
 * - 拾取框是绝对和平的，只要重叠就触发
 * 
 * @see EvolutionComponent - 进化组件（存储点数）
 * @see LootSpawnSystem - 掉落生成系统
 */
class PickupSystem {
public:
    void update(
        ComponentStore<EvolutionComponent>& evolutions,
        const ComponentStore<TransformComponent>& playerTransforms,
        ComponentStore<TransformComponent>& lootTransforms,
        ComponentStore<ItemDataComponent>& itemDatas,
        ComponentStore<PickupBoxComponent>& pickupBoxes)
    {
        // 遍历所有玩家（可能有多个）
        auto playerEntities = evolutions.entityList();
        for (Entity player : playerEntities) {
            if (!playerTransforms.has(player)) {
                continue;
            }
            
            const auto& playerTransform = playerTransforms.get(player);
            auto& evolution = evolutions.get(player);
            
            // 玩家碰撞框（40x40，中心对齐）
            Rect playerRect = {
                playerTransform.position.x - 20.0f,
                playerTransform.position.y - 20.0f,
                40.0f,
                40.0f
            };
            
            // 遍历所有掉落物
            auto lootEntities = itemDatas.entityList();
            for (Entity loot : lootEntities) {
                if (!lootTransforms.has(loot) || !pickupBoxes.has(loot)) {
                    continue;
                }
                
                const auto& lootTransform = lootTransforms.get(loot);
                const auto& pickupBox = pickupBoxes.get(loot);
                const auto& itemData = itemDatas.get(loot);
                
                // 掉落物碰撞框（中心对齐）
                Rect lootRect = {
                    lootTransform.position.x - pickupBox.width / 2.0f,
                    lootTransform.position.y - pickupBox.height / 2.0f,
                    pickupBox.width,
                    pickupBox.height
                };
                
                // AABB 碰撞检测
                if (playerRect.overlaps(lootRect)) {
                    // 拾取成功！
                    evolution.evolutionPoints += itemData.amount;
                    evolution.totalEarned += itemData.amount;
                    
                    // std::cout << "[Pickup] Player obtained " << itemData.amount 
                    //           << " evolution points! (Total: " << evolution.evolutionPoints << ")\n";  // ← 已禁用
                    
                    // 销毁掉落物实体
                    lootTransforms.remove(loot);
                    itemDatas.remove(loot);
                    pickupBoxes.remove(loot);
                }
            }
        }
    }
};

#pragma once
#include "../core/Component.h"
#include "../core/ECS.h"
#include "../components/Transform.h"
#include "../components/Evolution.h"
#include "../components/ItemData.h"
#include "../components/PickupBox.h"
#include "../math/Rect.h"
#include <vector>

/**
 * @brief 拾取判定系统
 * 
 * 职责：
 * - 监听玩家（Transform + Evolution）
 * - 监听掉落物（Transform + ItemData + PickupBox）
 * - 计算 AABB 碰撞
 * - 拾取成功：增加进化点数，延迟销毁掉落物
 * 
 * ⚠️ Bug 修复：
 * 1. 使用延迟销毁，避免迭代器失效
 * 2. 添加 alreadyMarked 检查，防止一帧多吃
 * 3. ECS& ecs 必须作为第一个参数
 * 
 * @see EvolutionComponent - 进化组件（存储点数）
 * @see LootSpawnSystem - 掉落生成系统
 */
class PickupSystem {
public:
    void update(
        ECS& ecs,  // ← 【关键】必须传入 ecs 引用（第一个参数）
        ComponentStore<EvolutionComponent>& evolutions,
        const ComponentStore<TransformComponent>& playerTransforms,
        ComponentStore<TransformComponent>& lootTransforms,
        ComponentStore<ItemDataComponent>& itemDatas,
        ComponentStore<PickupBoxComponent>& pickupBoxes,
        ComponentStore<MagnetComponent>& magnets)  // ← 新增：清理 MagnetComponent
    {
        // ← 【修复】延迟销毁名单
        std::vector<Entity> entitiesToDestroy;
        
        auto playerEntities = evolutions.entityList();
        for (Entity player : playerEntities) {
            if (!playerTransforms.has(player)) continue;
            
            const auto& pTrans = playerTransforms.get(player);
            auto& evo = evolutions.get(player);
            
            // 简单的 AABB 检查
            Rect playerRect = {
                pTrans.position.x - 20.0f,
                pTrans.position.y - 20.0f,
                40.0f,
                40.0f
            };
            
            auto lootEntities = itemDatas.entityList();
            for (Entity loot : lootEntities) {
                // ← 【关键排查】防止一帧多吃
                bool alreadyMarked = false;
                for (auto e : entitiesToDestroy) {
                    if (e == loot) {
                        alreadyMarked = true;
                        break;
                    }
                }
                if (alreadyMarked) continue;
                
                if (!lootTransforms.has(loot) || !pickupBoxes.has(loot)) continue;
                
                const auto& lTrans = lootTransforms.get(loot);
                const auto& pBox = pickupBoxes.get(loot);
                const auto& item = itemDatas.get(loot);
                
                Rect lootRect = {
                    lTrans.position.x - pBox.width / 2.0f,
                    lTrans.position.y - pBox.height / 2.0f,
                    pBox.width,
                    pBox.height
                };
                
                if (playerRect.overlaps(lootRect)) {
                    // 1. 先加点（确保在销毁前读取数据）
                    evo.evolutionPoints += item.amount;
                    evo.totalEarned += item.amount;
                    
                    // 2. 将实体加入行刑名单
                    entitiesToDestroy.push_back(loot);
                    
                    std::cout << "[Pickup] Collected item! Points: " << evo.evolutionPoints << std::endl;
                }
            }
        }
        
        // 3. 统一销毁：先移除所有组件，再销毁实体 ID
        for (Entity e : entitiesToDestroy) {
            // 先移除所有组件（防止幽灵数据）
            lootTransforms.remove(e);
            itemDatas.remove(e);
            pickupBoxes.remove(e);
            magnets.remove(e);  // ← 新增：清理 MagnetComponent
            
            // 再销毁实体 ID
            ecs.destroy(e);
        }
    }
};

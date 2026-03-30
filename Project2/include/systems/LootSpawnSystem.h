#pragma once
#include "../core/Component.h"
#include "../core/ECS.h"
#include "../components/Transform.h"
#include "../components/LootDrop.h"
#include "../components/ItemData.h"
#include "../components/PickupBox.h"
#include "../components/DeathTag.h"
#include <cstdlib>

/**
 * @brief 掉落生成系统
 * 
 * 职责：
 * - 监听拥有 LootDropComponent + DeathTag 的实体（刚死亡的怪物）
 * - 读取怪物的最后坐标
 * - 触发随机数判定 dropChance
 * - 生成掉落物实体（Transform + ItemData + PickupBox）
 * 
 * ⚠️ 时序安全：必须在 DeathSystem 之前执行！
 * 因为一旦 DeathSystem 执行了 ecs.destroy()，怪物的坐标就消失了。
 */
class LootSpawnSystem {
public:
    void update(
        ComponentStore<TransformComponent>& transforms,
        ComponentStore<LootDropComponent>& lootDrops,
        ComponentStore<ItemDataComponent>& itemDatas,
        ComponentStore<PickupBoxComponent>& pickupBoxes,
        const ComponentStore<DeathTag>& deathTags,
        ECS& ecs)
    {
        auto entities = deathTags.entityList();
        for (Entity entity : entities) {
            // 安全检查：必须有 LootDropComponent 和 TransformComponent
            if (!lootDrops.has(entity) || !transforms.has(entity)) {
                continue;
            }
            
            const auto& lootDrop = lootDrops.get(entity);
            const auto& transform = transforms.get(entity);
            
            // 如果已经掉落过，跳过（防抖锁）
            if (lootDrop.hasDropped) {
                continue;
            }
            
            // 遍历掉落表
            for (int i = 0; i < lootDrop.lootCount; i++) {
                const auto& entry = lootDrop.lootTable[i];
                
                // 随机数判定（0-1）
                float rand = static_cast<float>(std::rand()) / RAND_MAX;
                
                if (rand <= entry.dropChance) {
                    // 【修复】生成掉落物实体
                    Entity lootEntity = ecs.create();
                    
                    // 【修复】添加随机抖动 (±20 像素)
                    float jitterX = (static_cast<float>(std::rand()) / RAND_MAX) * 40.0f - 20.0f;
                    float jitterY = (static_cast<float>(std::rand()) / RAND_MAX) * 40.0f - 20.0f;
                    
                    // 【修复】Transform：必须使用死者的 position + 抖动！
                    transforms.add(lootEntity, {
                        .position = {transform.position.x + jitterX, transform.position.y + jitterY},
                        .scale = {1.0f, 1.0f},
                        .rotation = 0.0f,
                        .velocity = {0.0f, 0.0f}
                    });
                    
                    // 【修复】ItemData：物品数据
                    int amount = entry.minCount;
                    if (entry.maxCount > entry.minCount) {
                        amount += std::rand() % (entry.maxCount - entry.minCount + 1);
                    }
                    
                    itemDatas.add(lootEntity, {
                        .itemId = entry.itemId,
                        .amount = amount,
                        .isPickupable = true
                    });
                    
                    // 【修复】PickupBox：拾取碰撞框
                    pickupBoxes.add(lootEntity, {
                        .width = 20.0f,
                        .height = 20.0f
                    });
                }
            }
            
            // 【修复】标记已掉落（防止重复）
            LootDropComponent& mutableLootDrop = const_cast<LootDropComponent&>(lootDrop);
            mutableLootDrop.hasDropped = true;
        }
    }
};

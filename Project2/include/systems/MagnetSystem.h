#pragma once
#include "../core/Component.h"
#include "../components/Transform.h"
#include "../components/MagnetComponent.h"
#include "../components/ItemData.h"
#include <cmath>

/**
 * @brief 磁力吸附系统（方案 A：玩家拥有吸收半径）
 * 
 * 职责：
 * - 遍历所有带有 MagnetComponent 的玩家实体
 * - 计算玩家与所有掉落物的距离
 * - 如果掉落物在玩家吸收半径内且免疫时间结束，设置掉落物飞向玩家
 * - 如果脱离吸附范围，施加摩擦力减速
 * 
 * ⚠️ 关键设计：MagnetComponent 挂在玩家身上，而不是掉落物！
 * 
 * ⚠️ 时序：必须在 MovementSystem 之前执行！
 * 
 * @see MagnetComponent - 磁力组件（玩家吸收半径）
 * @see MovementSystem - 统一运动系统
 */
class MagnetSystem {
public:
    void update(
        ComponentStore<TransformComponent>& transforms,
        ComponentStore<MagnetComponent>& magnets,
        ComponentStore<TransformComponent>& lootTransforms,
        ComponentStore<ItemDataComponent>& itemDatas,
        float dt)  // ← 新增：帧时间，用于免疫计时器和摩擦力
    {
        // ← 【关键设计】遍历所有带有 MagnetComponent 的玩家
        auto magnetEntities = magnets.entityList();
        for (Entity player : magnetEntities) {
            // 防御性检查
            if (!transforms.has(player)) {
                continue;
            }
            
            const auto& playerTransform = transforms.get(player);
            const auto& playerMagnet = magnets.get(player);
            
            // ← 检查吸收半径是否有效（> 0）
            if (playerMagnet.magnetRadius <= 0.0f) {
                continue;
            }
            
            // ← 遍历所有掉落物
            auto lootEntities = itemDatas.entityList();
            for (Entity loot : lootEntities) {
                // 防御性检查
                if (!lootTransforms.has(loot)) {
                    continue;
                }
                
                auto& lootTransform = lootTransforms.get(loot);
                
                // ← 【修复 1】检查磁吸免疫计时器
                auto itemData = itemDatas.get(loot);
                if (itemData.magnetImmunityTimer > 0.0f) {
                    itemData.magnetImmunityTimer -= dt;
                    itemDatas.add(loot, itemData);  // 更新免疫时间
                    continue;  // 免疫期间不执行吸附
                }
                
                // 计算掉落物与玩家的距离
                float dx = playerTransform.position.x - lootTransform.position.x;
                float dy = playerTransform.position.y - lootTransform.position.y;
                float distance = std::sqrt(dx * dx + dy * dy);
                
                // 如果掉落物在玩家吸收半径内
                if (distance < playerMagnet.magnetRadius && distance > 0.0f) {
                    // 计算归一化向量（从掉落物指向玩家）
                    float dirX = dx / distance;
                    float dirY = dy / distance;
                    
                    // 设置掉落物速度（飞向玩家）
                    lootTransform.velocity.x = dirX * playerMagnet.magnetSpeed;
                    lootTransform.velocity.y = dirY * playerMagnet.magnetSpeed;
                } else {
                    // ← 【修复 2】脱离吸附范围，施加摩擦力减速
                    lootTransform.velocity.x *= 0.9f;
                    lootTransform.velocity.y *= 0.9f;
                }
            }
        }
    }
};

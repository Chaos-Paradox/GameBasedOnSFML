#pragma once
#include "../core/Component.h"
#include "../core/Entity.h"
#include "../core/ECS.h"
#include "../components/Lifetime.h"
#include "../components/Transform.h"
#include "../components/Hitbox.h"
#include "../components/MagnetComponent.h"
#include "../components/ItemData.h"
#include "../components/PickupBox.h"
#include "../components/DamageEventComponent.h"  // ← 新增：伤害事件清理
#include <vector>

/**
 * @brief 清理系统
 * 
 * 职责：
 * - 扫描所有 LifetimeComponent
 * - 减少 timeLeft
 * - 销毁 timeLeft <= 0 的实体
 * - 销毁所有 DamageEventComponent 事件实体（只活 1 帧）
 * 
 * @see DamageEventComponent - 伤害事件载荷（生命周期 1 帧）
 */
class CleanupSystem {
public:
    void update(
        ComponentStore<LifetimeComponent>& lifetimes,
        ComponentStore<TransformComponent>& transforms,
        ComponentStore<HitboxComponent>& hitboxes,
        ComponentStore<MagnetComponent>& magnets,
        ComponentStore<ItemDataComponent>& itemDatas,
        ComponentStore<PickupBoxComponent>& pickupBoxes,
        ComponentStore<DamageEventComponent>& damageEvents,  // ← 新增：伤害事件清理
        ECS& ecs,  // ← 传入 ECS 用于销毁实体
        float dt)
    {
        // 1. 清理 LifetimeComponent 实体
        auto entities = lifetimes.entityList();
        
        // 倒序遍历，安全删除
        for (int i = static_cast<int>(entities.size()) - 1; i >= 0; --i) {
            Entity entity = entities[i];
            
            // 安全检查：确保组件存在
            if (!lifetimes.has(entity)) {
                continue;
            }
            
            auto& lifetime = lifetimes.get(entity);
            lifetime.timeLeft -= dt;
            
            if (lifetime.timeLeft <= 0.0f && lifetime.autoDestroy) {
                // 1. 先销毁所有可能的组件（防止组件残留）
                // 注意：使用 has() 检查，因为组件可能已被其他系统移除
                if (transforms.has(entity)) transforms.remove(entity);
                if (hitboxes.has(entity)) hitboxes.remove(entity);
                if (lifetimes.has(entity)) lifetimes.remove(entity);
                if (magnets.has(entity)) magnets.remove(entity);
                if (itemDatas.has(entity)) itemDatas.remove(entity);
                if (pickupBoxes.has(entity)) pickupBoxes.remove(entity);
                
                // 2. 再销毁实体 ID
                ecs.destroy(entity);
            }
        }
        
        // ← 【新增】2. 清理 DamageEventComponent 事件实体（只活 1 帧）
        auto eventEntities = damageEvents.entityList();
        for (Entity eventEntity : eventEntities) {
            // 销毁事件实体及其组件
            if (damageEvents.has(eventEntity)) {
                damageEvents.remove(eventEntity);
            }
            ecs.destroy(eventEntity);
        }
    }
};

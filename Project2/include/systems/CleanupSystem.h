#pragma once
#include "../core/Component.h"
#include "../core/Entity.h"
#include "../components/Lifetime.h"
#include "../components/Transform.h"
#include "../components/Hitbox.h"
#include <vector>

/**
 * @brief 清理系统
 * 
 * 职责：
 * - 扫描所有 LifetimeComponent
 * - 减少 timeLeft
 * - 销毁 timeLeft <= 0 的实体
 */
class CleanupSystem {
public:
    void update(
        ComponentStore<LifetimeComponent>& lifetimes,
        ComponentStore<TransformComponent>& transforms,
        ComponentStore<HitboxComponent>& hitboxes,
        float dt)
    {
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
                // 销毁所有相关组件
                transforms.remove(entity);
                hitboxes.remove(entity);
                lifetimes.remove(entity);
            }
        }
    }
};

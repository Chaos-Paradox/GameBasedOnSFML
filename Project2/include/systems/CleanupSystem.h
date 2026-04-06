#pragma once
#include "../core/Component.h"
#include "../core/ECS.h"
#include "../components/DeathTag.h"
#include "../components/Lifetime.h"
#include <vector>

/**
 * @brief 清理系统（延迟销毁）
 * 
 * ⚠️ 架构设计：
 * - 在帧末统一清理所有带 DeathTag 的实体
 * - 清理到期的 Lifetime 实体
 * - 防止 std::out_of_range 崩溃
 * 
 * 执行时机：主循环最末尾（渲染之后）
 * 
 * @see DeathTag - 死亡标记
 * @see LifetimeComponent - 寿命组件
 */
class CleanupSystem {
public:
    void update(
        ECS& ecs,
        ComponentStore<DeathTag>& deathTags,
        ComponentStore<LifetimeComponent>& lifetimes,
        float dt)
    {
        // ========== 1. 清理 DeathTag 实体 ==========
        auto deathEntities = deathTags.entityList();
        
        // 复制列表，避免迭代中修改
        std::vector<Entity> toDestroy(deathEntities.begin(), deathEntities.end());
        
        for (Entity entity : toDestroy) {
            ecs.destroy(entity);
        }
        
        // ========== 2. 清理到期 Lifetime 实体 ==========
        auto lifetimeEntities = lifetimes.entityList();
        
        for (Entity entity : lifetimeEntities) {
            if (!lifetimes.has(entity)) continue;
            
            auto& lifetime = lifetimes.get(entity);
            lifetime.timeLeft -= dt;
            
            if (lifetime.timeLeft <= 0.0f) {
                if (lifetime.autoDestroy) {
                    ecs.destroy(entity);
                }
            }
        }
    }
};

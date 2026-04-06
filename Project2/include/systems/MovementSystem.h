#pragma once
#include "../core/Component.h"
#include "../components/Transform.h"
#include "../components/ItemData.h"
#include <cmath>

class MovementSystem {
public:
    void update(
        ComponentStore<TransformComponent>& transforms,
        ComponentStore<ItemDataComponent>& itemDatas,  // ← 新增：用于识别掉落物
        float dt)
    {
        auto entities = transforms.entityList();
        for (Entity entity : entities) {
            auto& transform = transforms.get(entity);
            
            // ← 【核心修复】掉落物阻尼（每帧衰减）
            if (itemDatas.has(entity)) {
                // pow(0.01f, dt) ≈ 0.955 (dt=0.0166667)
                // 0.5 秒内速度衰减到约 3%
                float damping = std::pow(0.01f, dt);
                transform.velocity.x *= damping;
                transform.velocity.y *= damping;
            }
            
            // 应用位移（先加速度再乘 dt）
            transform.position.x += transform.velocity.x * dt;
            transform.position.y += transform.velocity.y * dt;
        }
    }
};

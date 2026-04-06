#pragma once
#include "../core/Component.h"
#include "../core/ECS.h"
#include "../components/ColliderComponent.h"
#include "../components/Transform.h"
#include <cmath>
#include <iostream>

/**
 * @brief 物理碰撞系统（圆柱体排斥）
 * 
 * ⚠️ 架构设计：
 * - 独立于战斗碰撞系统（Hitbox/Hurtbox）
 * - 用于实体间的物理阻挡（推挤、碰撞）
 * - Kinematic Separation：直接修改 position，不修改 velocity
 * 
 * 算法：
 * 1. 遍历所有 Collider，两两对比（O(N²)）
 * 2. 计算 XY 平面距离 dist
 * 3. 如果 dist < (radiusA + radiusB)，发生重叠
 * 4. 计算推力方向 dir = normalize(posA - posB)
 * 5. 根据 isStatic 状态分配排斥距离
 * 
 * @see ColliderComponent - 圆柱体碰撞器
 */
class PhysicalCollisionSystem {
public:
    void update(
        ComponentStore<ColliderComponent>& colliders,
        ComponentStore<TransformComponent>& transforms,
        float dt)
    {
        (void)dt;
        
        auto colliderEntities = colliders.entityList();
        
        // 两两对比（O(N²) 简单检测）
        for (size_t i = 0; i < colliderEntities.size(); ++i) {
            Entity entityA = colliderEntities[i];
            if (!colliders.has(entityA) || !transforms.has(entityA)) continue;
            
            auto& colliderA = colliders.get(entityA);
            auto& transformA = transforms.get(entityA);
            
            for (size_t j = i + 1; j < colliderEntities.size(); ++j) {
                Entity entityB = colliderEntities[j];
                if (!colliders.has(entityB) || !transforms.has(entityB)) continue;
                
                auto& colliderB = colliders.get(entityB);
                auto& transformB = transforms.get(entityB);
                
                // ========== 圆形碰撞检测 ==========
                float dx = transformB.position.x - transformA.position.x;
                float dy = transformB.position.y - transformA.position.y;
                float dist = std::sqrt(dx * dx + dy * dy);
                
                float minDist = colliderA.radius + colliderB.radius;
                
                // 没有重叠，跳过
                if (dist >= minDist) continue;
                
                // 距离太小，避免除以零
                if (dist < 0.001f) {
                    // 随机方向推开
                    dx = 1.0f;
                    dy = 0.0f;
                    dist = 0.001f;
                }
                
                // ========== 计算排斥 ==========
                float overlap = minDist - dist;  // 重叠深度
                
                // 归一化推力方向（从 B 指向 A）
                float invDist = 1.0f / dist;
                float dirX = dx * invDist;
                float dirY = dy * invDist;
                
                // ========== 分离逻辑 (Kinematic Separation) ==========
                if (colliderA.isStatic && colliderB.isStatic) {
                    // 两个都是静态，都不动
                    continue;
                }
                else if (colliderA.isStatic) {
                    // A 是静态，B 被推开全部距离
                    transformB.position.x += dirX * overlap;
                    transformB.position.y += dirY * overlap;
                    
                    std::cout << "[Physics] Static collision: B pushed by " << overlap << "\n";
                }
                else if (colliderB.isStatic) {
                    // B 是静态，A 被推开全部距离
                    transformA.position.x -= dirX * overlap;
                    transformA.position.y -= dirY * overlap;
                    
                    std::cout << "[Physics] Static collision: A pushed by " << overlap << "\n";
                }
                else {
                    // 两个都是动态，各推开一半
                    float separation = overlap * 0.5f;
                    
                    transformA.position.x -= dirX * separation;
                    transformA.position.y -= dirY * separation;
                    transformB.position.x += dirX * separation;
                    transformB.position.y += dirY * separation;
                    
                    std::cout << "[Physics] Dynamic collision: separation=" << separation << "\n";
                }
            }
        }
    }
};

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
                
                // 🚀 极小值保护：如果距离为 0（完美重合），强行赋予一个微小的随机偏移！
                // 防止 0/0 = NaN 导致实体消失
                if (dist == 0.0f) {
                    dx = 0.001f;
                    dy = 0.001f;
                    dist = std::sqrt(dx * dx + dy * dy);
                    std::cout << "[Physics] ⚠️ Zero distance detected! Applied epsilon offset.\n";
                }
                
                float minDist = colliderA.radius + colliderB.radius;
                
                // 没有重叠，跳过
                if (dist >= minDist) continue;
                
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
                    // 两个都是动态，按质量权重分配排斥距离
                    // 质量越大，移动越少；质量越小，移动越多
                    float totalMass = colliderA.mass + colliderB.mass;
                    float ratioA = colliderB.mass / totalMass;  // A 的移动比例由 B 的质量决定
                    float ratioB = colliderA.mass / totalMass;  // B 的移动比例由 A 的质量决定
                    
                    transformA.position.x -= dirX * overlap * ratioA;
                    transformA.position.y -= dirY * overlap * ratioA;
                    transformB.position.x += dirX * overlap * ratioB;
                    transformB.position.y += dirY * overlap * ratioB;
                    
                    std::cout << "[Physics] Dynamic collision (mass): A moves " << (overlap * ratioA)
                              << ", B moves " << (overlap * ratioB) << "\n";
                    
                    // ========== 动量交换 (Momentum Impulse) ==========
                    // 1. 计算相对速度
                    float relVelX = transformA.velocity.x - transformB.velocity.x;
                    float relVelY = transformA.velocity.y - transformB.velocity.y;
                    
                    // 2. 计算沿法线（推力方向 dir）的速度
                    float velocityAlongNormal = relVelX * dirX + relVelY * dirY;
                    
                    // 如果两个物体已经在互相远离，不要进行动量处理
                    if (velocityAlongNormal > 0) {
                        continue;
                    }
                    
                    // 3. 弹性系数 (Restitution) - 0.5f 表示中等弹力
                    float e = 0.5f;
                    
                    // 4. 计算冲量标量 (Impulse scalar)
                    float j = -(1.0f + e) * velocityAlongNormal;
                    j /= (1.0f / colliderA.mass + 1.0f / colliderB.mass);
                    
                    // 5. 应用冲量，修改实际的 Velocity！
                    float impulseX = j * dirX;
                    float impulseY = j * dirY;
                    
                    if (!colliderA.isStatic) {
                        transformA.velocity.x += (impulseX / colliderA.mass);
                        transformA.velocity.y += (impulseY / colliderA.mass);
                    }
                    if (!colliderB.isStatic) {
                        transformB.velocity.x -= (impulseX / colliderB.mass);
                        transformB.velocity.y -= (impulseY / colliderB.mass);
                    }
                    
                    std::cout << "[Physics] 💥 Momentum impulse! j=" << j 
                              << " A.vel=(" << transformA.velocity.x << "," << transformA.velocity.y << ")"
                              << " B.vel=(" << transformB.velocity.x << "," << transformB.velocity.y << ")\n";
                }
            }
        }
    }
};

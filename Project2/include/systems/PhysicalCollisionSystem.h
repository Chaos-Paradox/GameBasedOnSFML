#pragma once
#include "core/GameWorld.h"
#include <cmath>
#include <iostream>

/**
 * @brief 物理碰撞系统（圆柱体排斥 + CCD 连续碰撞检测）
 *
 * 质量数据来源：MomentumComponent::mass（不再从 ColliderComponent 读取）
 * 速度数据来源：优先使用 MomentumComponent::velocity，回退到 TransformComponent::velocity
 *
 * CCD 连续碰撞检测：
 *   - 对 useCCD=true 的高速实体（如炸弹），用 prevPos → currentPos 轨迹做射线检测
 *   - 检测轨迹上与静态碰撞体的最近点，如果距离 < radiusA + radiusB 则判定碰撞
 *   - 碰撞后将实体推回到碰撞点，防止隧穿
 */
class PhysicalCollisionSystem {
public:
    void update(GameWorld& world, float dt)
    {
        (void)dt;

        // ========== 记录上一帧位置（用于 CCD） ==========
        auto momentumEntities = world.momentums.entityList();
        for (Entity e : momentumEntities) {
            if (world.momentums.has(e) && world.transforms.has(e)) {
                auto& mom = world.momentums.get(e);
                auto& trans = world.transforms.get(e);
                mom.prevPosX = trans.position.x;
                mom.prevPosY = trans.position.y;
            }
        }

        // ========== 常规碰撞检测（离散） ==========
        auto colliderEntities = world.colliders.entityList();

        for (size_t i = 0; i < colliderEntities.size(); ++i) {
            Entity entityA = colliderEntities[i];
            if (!world.colliders.has(entityA) || !world.transforms.has(entityA)) continue;

            auto& colliderA = world.colliders.get(entityA);
            auto& transformA = world.transforms.get(entityA);

            for (size_t j = i + 1; j < colliderEntities.size(); ++j) {
                Entity entityB = colliderEntities[j];
                if (!world.colliders.has(entityB) || !world.transforms.has(entityB)) continue;

                auto& colliderB = world.colliders.get(entityB);
                auto& transformB = world.transforms.get(entityB);

                // 炸弹被 Dash 踢飞时，跳过 PhysicalCollisionSystem 的处理
                // 只由 BombSystem 的 CCD 处理，避免物理推开干扰连续踢
                bool aIsBomb = world.bombs.has(entityA);
                bool bIsBomb = world.bombs.has(entityB);
                if (aIsBomb || bIsBomb) {
                    bool aIsDashing = world.states.has(entityA) &&
                                      world.states.get(entityA).currentState == CharacterState::Dash;
                    bool bIsDashing = world.states.has(entityB) &&
                                      world.states.get(entityB).currentState == CharacterState::Dash;
                    if (aIsDashing || bIsDashing) continue;
                }

                float dx = transformB.position.x - transformA.position.x;
                float dy = transformB.position.y - transformA.position.y;
                float dist = std::sqrt(dx * dx + dy * dy);

                if (dist == 0.0f) {
                    dx = 0.001f;
                    dy = 0.001f;
                    dist = std::sqrt(dx * dx + dy * dy);
                    std::cout << "[Physics] ⚠️ Zero distance detected! Applied epsilon offset.\n";
                }

                float minDist = colliderA.radius + colliderB.radius;
                if (dist >= minDist) continue;

                // Z 轴过滤：跳跃中的实体不应与地面实体发生物理排斥
                float zA = world.zTransforms.has(entityA) ? world.zTransforms.get(entityA).z : 0.0f;
                float hA = world.zTransforms.has(entityA) ? world.zTransforms.get(entityA).height : 40.0f;
                float zB = world.zTransforms.has(entityB) ? world.zTransforms.get(entityB).z : 0.0f;
                float hB = world.zTransforms.has(entityB) ? world.zTransforms.get(entityB).height : 40.0f;
                if (zA > zB + hB || zA + hA < zB) continue;  // Z 轴不相交，跳过

                float overlap = minDist - dist;
                float invDist = 1.0f / dist;
                float dirX = dx * invDist;
                float dirY = dy * invDist;

                // 质量从 MomentumComponent 读取
                float massA = 1.0f;
                float massB = 1.0f;
                if (world.momentums.has(entityA)) massA = world.momentums.get(entityA).mass;
                if (world.momentums.has(entityB)) massB = world.momentums.get(entityB).mass;

                if (colliderA.isStatic && colliderB.isStatic) {
                    continue;
                }
                else if (colliderA.isStatic) {
                    transformB.position.x += dirX * overlap;
                    transformB.position.y += dirY * overlap;
                    std::cout << "[Physics] Static collision: B pushed by " << overlap << "\n";
                }
                else if (colliderB.isStatic) {
                    transformA.position.x -= dirX * overlap;
                    transformA.position.y -= dirY * overlap;
                    std::cout << "[Physics] Static collision: A pushed by " << overlap << "\n";
                }
                else {
                    float totalMass = massA + massB;
                    float ratioA = massB / totalMass;
                    float ratioB = massA / totalMass;

                    transformA.position.x -= dirX * overlap * ratioA;
                    transformA.position.y -= dirY * overlap * ratioA;
                    transformB.position.x += dirX * overlap * ratioB;
                    transformB.position.y += dirY * overlap * ratioB;

                    std::cout << "[Physics] Dynamic collision (mass): A moves " << (overlap * ratioA)
                              << ", B moves " << (overlap * ratioB) << "\n";

                    // 速度从 MomentumComponent 读取，回退到 TransformComponent
                    Vec2 velA = world.momentums.has(entityA) ? world.momentums.get(entityA).velocity
                                                             : transformA.velocity;
                    Vec2 velB = world.momentums.has(entityB) ? world.momentums.get(entityB).velocity
                                                             : transformB.velocity;

                    float relVelX = velA.x - velB.x;
                    float relVelY = velA.y - velB.y;
                    float velocityAlongNormal = relVelX * dirX + relVelY * dirY;

                    if (velocityAlongNormal > 0) continue;

                    float e = 0.5f;
                    float j = -(1.0f + e) * velocityAlongNormal;
                    j /= (1.0f / massA + 1.0f / massB);

                    float impulseX = j * dirX;
                    float impulseY = j * dirY;

                    float newVxA = velA.x + (impulseX / massA);
                    float newVyA = velA.y + (impulseY / massA);
                    float newVxB = velB.x - (impulseX / massB);
                    float newVyB = velB.y - (impulseY / massB);

                    transformA.velocity.x = newVxA;
                    transformA.velocity.y = newVyA;
                    transformB.velocity.x = newVxB;
                    transformB.velocity.y = newVyB;

                    if (world.momentums.has(entityA)) {
                        world.momentums.get(entityA).velocity = {newVxA, newVyA};
                    }
                    if (world.momentums.has(entityB)) {
                        world.momentums.get(entityB).velocity = {newVxB, newVyB};
                    }

                    std::cout << "[Physics] 💥 Momentum impulse! j=" << j << "\n";
                }
            }
        }

        // ========== CCD 连续碰撞检测（防止高速实体隧穿） ==========
        resolveCCD(world);
    }

private:
    /**
     * @brief CCD 连续碰撞检测
     *
     * 对 useCCD=true 的实体，用 prevPos → currentPos 轨迹做线段-圆最近点检测。
     * 如果轨迹上与静态碰撞体的最近距离 < radiusA + radiusB，则将实体推回到碰撞点。
     */
    void resolveCCD(GameWorld& world)
    {
        auto momentumEntities = world.momentums.entityList();

        for (Entity entityA : momentumEntities) {
            if (!world.momentums.has(entityA) || !world.transforms.has(entityA)) continue;

            auto& momA = world.momentums.get(entityA);
            if (!momA.useCCD) continue;

            auto& transformA = world.transforms.get(entityA);

            // 轨迹线段：prevPos → currentPos
            float segX = transformA.position.x - momA.prevPosX;
            float segY = transformA.position.y - momA.prevPosY;
            float segLenSq = segX * segX + segY * segY;

            // 如果实体几乎没有移动，跳过 CCD
            if (segLenSq < 0.001f) continue;

            auto colliderEntities = world.colliders.entityList();

            for (Entity entityB : colliderEntities) {
                if (entityA == entityB) continue;
                if (!world.colliders.has(entityB)) continue;

                auto& colliderB = world.colliders.get(entityB);
                // 只需要检测与静态碰撞体的 CCD（围栏等）
                if (!colliderB.isStatic) continue;

                auto& transformB = world.transforms.get(entityB);

                // 计算线段上离 colliderB 中心最近的点
                float tx = transformB.position.x - momA.prevPosX;
                float ty = transformB.position.y - momA.prevPosY;
                float t = (tx * segX + ty * segY) / segLenSq;
                t = t < 0.0f ? 0.0f : (t > 1.0f ? 1.0f : t);

                float closestX = momA.prevPosX + t * segX;
                float closestY = momA.prevPosY + t * segY;

                float dx = transformB.position.x - closestX;
                float dy = transformB.position.y - closestY;
                float dist = std::sqrt(dx * dx + dy * dy);

                float minDist = colliderB.radius;
                // 对于动态实体 A，用 MomentumComponent 的 radius 回退
                float radiusA = 1.0f;
                if (world.colliders.has(entityA)) {
                    radiusA = world.colliders.get(entityA).radius;
                }
                minDist += radiusA;

                if (dist < minDist && dist > 0.001f) {
                    // 碰撞！推回到碰撞点
                    float pushBackX = closestX + (dx / dist) * (minDist - 0.01f);
                    float pushBackY = closestY + (dy / dist) * (minDist - 0.01f);

                    transformA.position.x = pushBackX;
                    transformA.position.y = pushBackY;

                    // 速度反射（沿碰撞法线）
                    float nx = dx / dist;
                    float ny = dy / dist;

                    Vec2 vel = transformA.velocity;
                    float dotVN = vel.x * nx + vel.y * ny;

                    // 只反射朝向碰撞体的速度分量
                    if (dotVN < 0.0f) {
                        transformA.velocity.x -= 2.0f * dotVN * nx;
                        transformA.velocity.y -= 2.0f * dotVN * ny;

                        // 同步 momentum 速度
                        momA.velocity = {transformA.velocity.x, transformA.velocity.y};
                    }

                    std::cout << "[Physics/CCD] ⚡ Tunneling prevented! entity=" << (uint32_t)entityA
                              << " pushed to (" << pushBackX << ", " << pushBackY << ")\n";

                    // 一个 CCD 实体每帧只处理一次碰撞
                    break;
                }
            }
        }
    }
};

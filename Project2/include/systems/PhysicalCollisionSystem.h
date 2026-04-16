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

                    // 炸弹刚被踢飞（collisionCooldown > 0）也跳过，避免 dash 过程中被物理推开
                    Entity bombEntity = aIsBomb ? entityA : entityB;
                    if (world.momentums.has(bombEntity) &&
                        world.momentums.get(bombEntity).collisionCooldown > 0.0f) continue;

                    // 炸弹速度 > 100px/s 表示正在飞行中，也跳过物理碰撞
                    auto& bombTrans = aIsBomb ? transformA : transformB;
                    float bombSpeed = std::sqrt(bombTrans.velocity.x * bombTrans.velocity.x +
                                                bombTrans.velocity.y * bombTrans.velocity.y);
                    if (bombSpeed > 100.0f) continue;
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

                // ========== 踢飞豁免期拦截 (Collision Immunity) ==========
                // 炸弹被踢飞后，在豁免期内无视原踢飞者的刚体碰撞，直接穿透
                {
                    bool skipA = world.throwables.has(entityA) &&
                                 world.throwables.get(entityA).lastKickedBy == entityB &&
                                 world.throwables.get(entityA).ignoreKickerTimer > 0.0f;
                    bool skipB = world.throwables.has(entityB) &&
                                 world.throwables.get(entityB).lastKickedBy == entityA &&
                                 world.throwables.get(entityB).ignoreKickerTimer > 0.0f;
                    if (skipA || skipB) continue;
                }

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

                    // 逃逸速度：推开后必须有足够速度远离，否则下一帧摩擦衰减又重叠
                    float friction = std::pow(0.001f, dt);
                    float escapeSpeed = overlap / dt / friction;
                    if (world.momentums.has(entityA)) {
                        auto& momA = world.momentums.get(entityA);
                        momA.velocity.x -= dirX * escapeSpeed;
                        momA.velocity.y -= dirY * escapeSpeed;
                        transformA.velocity = {momA.velocity.x, momA.velocity.y};
                    }

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

                    // ========== 重叠碰撞：只做位置推开 + 逃逸速度 ==========
                    // 推开后必须给足够速度远离，否则下一帧 MovementSystem 摩擦衰减
                    // (pow(0.001,dt)≈0.77x) 把推开效果抵消，导致玩家追上再次重叠。
                    // 逃逸速度 = 位移/dt/摩擦，保证衰减后的有效速度 = 推开位移/dt。
                    float friction = std::pow(0.001f, dt);
                    {
                        float dispA = overlap * ratioA;
                        float escapeSpeed = dispA / dt / friction;
                        if (world.momentums.has(entityA)) {
                            auto& momA = world.momentums.get(entityA);
                            momA.velocity.x -= dirX * escapeSpeed;
                            momA.velocity.y -= dirY * escapeSpeed;
                            transformA.velocity = {momA.velocity.x, momA.velocity.y};
                        }
                    }
                    {
                        float dispB = overlap * ratioB;
                        float escapeSpeed = dispB / dt / friction;
                        if (world.momentums.has(entityB)) {
                            auto& momB = world.momentums.get(entityB);
                            momB.velocity.x += dirX * escapeSpeed;
                            momB.velocity.y += dirY * escapeSpeed;
                            transformB.velocity = {momB.velocity.x, momB.velocity.y};
                        }
                    }
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

                // Z 轴过滤：CCD 同样需要检查 Z 轴相交
                float zA = world.zTransforms.has(entityA) ? world.zTransforms.get(entityA).z : 0.0f;
                float hA = world.zTransforms.has(entityA) ? world.zTransforms.get(entityA).height : 40.0f;
                float zB = world.zTransforms.has(entityB) ? world.zTransforms.get(entityB).z : 0.0f;
                float hB = world.zTransforms.has(entityB) ? world.zTransforms.get(entityB).height : 40.0f;
                if (zA > zB + hB || zA + hA < zB) continue;  // Z 轴不相交，跳过

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
                    // ========== 踢飞豁免期拦截 (CCD) ==========
                    bool skipA = world.throwables.has(entityA) &&
                                 world.throwables.get(entityA).lastKickedBy == entityB &&
                                 world.throwables.get(entityA).ignoreKickerTimer > 0.0f;
                    bool skipB = world.throwables.has(entityB) &&
                                 world.throwables.get(entityB).lastKickedBy == entityA &&
                                 world.throwables.get(entityB).ignoreKickerTimer > 0.0f;
                    if (skipA || skipB) continue;

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

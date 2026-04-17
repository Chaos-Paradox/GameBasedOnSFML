#pragma once
#include "core/GameWorld.h"
#include <cmath>
#include <iostream>
#include <unordered_map>

/**
 * @brief 物理碰撞系统（圆柱体排斥 + CCD 连续碰撞检测）
 *
 * ⚠️ 重构（ECS 纯净原则）：
 * - collisionCooldown → 由系统内部 collisionCooldowns 映射维护
 * - prevPosX/Y → 由系统内部 prevPositions 映射维护
 * - 质量数据来源：MomentumComponent::mass
 * - 速度数据来源：优先使用 MomentumComponent::velocity，回退到 TransformComponent::velocity
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

        // 碰撞冷却递减（系统内部维护）
        auto momentumEntities = world.momentums.entityList();
        for (Entity e : momentumEntities) {
            auto it = collisionCooldowns.find(e);
            if (it != collisionCooldowns.end() && it->second > 0.0f) {
                it->second -= dt;
                if (it->second < 0.0f) it->second = 0.0f;
            }
        }

        // ========== 记录上一帧位置（用于 CCD，系统内部维护） ==========
        auto allEntities = world.transforms.entityList();
        for (Entity e : allEntities) {
            if (world.transforms.has(e)) {
                auto& trans = world.transforms.get(e);
                prevPositions[e] = {trans.position.x, trans.position.y};
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

                // 炸弹被 Dash 踢飞时，跳过 PhysicalCollisionSystem
                bool aIsBomb = world.bombs.has(entityA);
                bool bIsBomb = world.bombs.has(entityB);
                if (aIsBomb || bIsBomb) {
                    bool aIsDashing = world.states.has(entityA) &&
                                      world.states.get(entityA).currentState == CharacterState::Dash;
                    bool bIsDashing = world.states.has(entityB) &&
                                      world.states.get(entityB).currentState == CharacterState::Dash;
                    if (aIsDashing || bIsDashing) continue;

                    // 碰撞冷却检查（系统内部映射）
                    Entity bombEntity = aIsBomb ? entityA : entityB;
                    auto cdIt = collisionCooldowns.find(bombEntity);
                    if (cdIt != collisionCooldowns.end() && cdIt->second > 0.0f) continue;

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

                // ========== 踢飞豁免期拦截 ==========
                {
                    bool skipA = world.throwables.has(entityA) &&
                                 world.throwables.get(entityA).lastKickedBy == entityB &&
                                 world.throwables.get(entityA).ignoreKickerTimer > 0.0f;
                    bool skipB = world.throwables.has(entityB) &&
                                 world.throwables.get(entityB).lastKickedBy == entityA &&
                                 world.throwables.get(entityB).ignoreKickerTimer > 0.0f;
                    if (skipA || skipB) continue;
                }

                // Z 轴过滤
                float zA = world.zTransforms.has(entityA) ? world.zTransforms.get(entityA).z : 0.0f;
                float hA = world.zTransforms.has(entityA) ? world.zTransforms.get(entityA).height : 40.0f;
                float zB = world.zTransforms.has(entityB) ? world.zTransforms.get(entityB).z : 0.0f;
                float hB = world.zTransforms.has(entityB) ? world.zTransforms.get(entityB).height : 40.0f;
                if (zA > zB + hB || zA + hA < zB) continue;

                float overlap = minDist - dist;
                float invDist = 1.0f / dist;
                float dirX = dx * invDist;
                float dirY = dy * invDist;

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

        // ========== CCD 连续碰撞检测 ==========
        resolveCCD(world);
    }

private:
    std::unordered_map<Entity, float> collisionCooldowns;
    std::unordered_map<Entity, Vec2> prevPositions;

    void resolveCCD(GameWorld& world)
    {
        auto momentumEntities = world.momentums.entityList();

        for (Entity entityA : momentumEntities) {
            if (!world.momentums.has(entityA) || !world.transforms.has(entityA)) continue;

            auto& momA = world.momentums.get(entityA);
            if (!momA.useCCD) continue;

            auto& transformA = world.transforms.get(entityA);

            // 使用系统内部保存的 prevPositions
            auto it = prevPositions.find(entityA);
            if (it == prevPositions.end()) continue;

            float prevX = it->second.x;
            float prevY = it->second.y;

            float segX = transformA.position.x - prevX;
            float segY = transformA.position.y - prevY;
            float segLenSq = segX * segX + segY * segY;

            if (segLenSq < 0.001f) continue;

            auto colliderEntities = world.colliders.entityList();

            for (Entity entityB : colliderEntities) {
                if (entityA == entityB) continue;
                if (!world.colliders.has(entityB)) continue;

                auto& colliderB = world.colliders.get(entityB);
                if (!colliderB.isStatic) continue;

                auto& transformB = world.transforms.get(entityB);

                float zA = world.zTransforms.has(entityA) ? world.zTransforms.get(entityA).z : 0.0f;
                float hA = world.zTransforms.has(entityA) ? world.zTransforms.get(entityA).height : 40.0f;
                float zB = world.zTransforms.has(entityB) ? world.zTransforms.get(entityB).z : 0.0f;
                float hB = world.zTransforms.has(entityB) ? world.zTransforms.get(entityB).height : 40.0f;
                if (zA > zB + hB || zA + hA < zB) continue;

                float tx = transformB.position.x - prevX;
                float ty = transformB.position.y - prevY;
                float t = (tx * segX + ty * segY) / segLenSq;
                t = t < 0.0f ? 0.0f : (t > 1.0f ? 1.0f : t);

                float closestX = prevX + t * segX;
                float closestY = prevY + t * segY;

                float dx = transformB.position.x - closestX;
                float dy = transformB.position.y - closestY;
                float dist = std::sqrt(dx * dx + dy * dy);

                float minDist = colliderB.radius;
                float radiusA = 1.0f;
                if (world.colliders.has(entityA)) {
                    radiusA = world.colliders.get(entityA).radius;
                }
                minDist += radiusA;

                if (dist < minDist && dist > 0.001f) {
                    bool skipA = world.throwables.has(entityA) &&
                                 world.throwables.get(entityA).lastKickedBy == entityB &&
                                 world.throwables.get(entityA).ignoreKickerTimer > 0.0f;
                    bool skipB = world.throwables.has(entityB) &&
                                 world.throwables.get(entityB).lastKickedBy == entityA &&
                                 world.throwables.get(entityB).ignoreKickerTimer > 0.0f;
                    if (skipA || skipB) continue;

                    float pushBackX = closestX + (dx / dist) * (minDist - 0.01f);
                    float pushBackY = closestY + (dy / dist) * (minDist - 0.01f);

                    transformA.position.x = pushBackX;
                    transformA.position.y = pushBackY;

                    float nx = dx / dist;
                    float ny = dy / dist;

                    Vec2 vel = transformA.velocity;
                    float dotVN = vel.x * nx + vel.y * ny;

                    if (dotVN < 0.0f) {
                        transformA.velocity.x -= 2.0f * dotVN * nx;
                        transformA.velocity.y -= 2.0f * dotVN * ny;
                        momA.velocity = {transformA.velocity.x, transformA.velocity.y};
                    }

                    std::cout << "[Physics/CCD] ⚡ Tunneling prevented! entity=" << (uint32_t)entityA
                              << " pushed to (" << pushBackX << ", " << pushBackY << ")\n";

                    break;
                }
            }
        }
    }
};

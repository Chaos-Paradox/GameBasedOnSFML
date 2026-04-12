#pragma once
#include "core/GameWorld.h"
#include <cmath>
#include <iostream>

/**
 * @brief 物理碰撞系统（圆柱体排斥）
 */
class PhysicalCollisionSystem {
public:
    void update(GameWorld& world, float dt)
    {
        (void)dt;

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

                float overlap = minDist - dist;
                float invDist = 1.0f / dist;
                float dirX = dx * invDist;
                float dirY = dy * invDist;

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
                    float totalMass = colliderA.mass + colliderB.mass;
                    float ratioA = colliderB.mass / totalMass;
                    float ratioB = colliderA.mass / totalMass;

                    transformA.position.x -= dirX * overlap * ratioA;
                    transformA.position.y -= dirY * overlap * ratioA;
                    transformB.position.x += dirX * overlap * ratioB;
                    transformB.position.y += dirY * overlap * ratioB;

                    std::cout << "[Physics] Dynamic collision (mass): A moves " << (overlap * ratioA)
                              << ", B moves " << (overlap * ratioB) << "\n";

                    float relVelX = transformA.velocity.x - transformB.velocity.x;
                    float relVelY = transformA.velocity.y - transformB.velocity.y;
                    float velocityAlongNormal = relVelX * dirX + relVelY * dirY;

                    if (velocityAlongNormal > 0) continue;

                    float e = 0.5f;
                    float j = -(1.0f + e) * velocityAlongNormal;
                    j /= (1.0f / colliderA.mass + 1.0f / colliderB.mass);

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

                    std::cout << "[Physics] 💥 Momentum impulse! j=" << j << "\n";
                }
            }
        }
    }
};

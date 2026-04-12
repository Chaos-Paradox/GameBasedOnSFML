#pragma once
#include "core/GameWorld.h"
#include <cmath>
#include <iostream>

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

/**
 * @brief 攻击判定系统（扇形几何瞬时扫描）
 */
class AttackSystem {
public:
    void update(GameWorld& world, float dt)
    {
        auto entities = world.attackStates.entityList();
        for (Entity entity : entities) {
            auto& state = world.states.get(entity);
            auto& attackState = world.attackStates.get(entity);

            if (state.currentState == CharacterState::Dash ||
                state.currentState == CharacterState::Hurt ||
                state.currentState == CharacterState::Dead) {
                continue;
            }

            if (state.currentState == CharacterState::Attack) {
                attackState.hitTimer -= dt;

                if (attackState.hitTimer <= 0.0f) {
                    state.currentState = CharacterState::Idle;
                    state.previousState = CharacterState::Idle;
                    state.stateTimer = 0.0f;
                    attackState.hasFiredDamage = false;
                    std::cout << "[AttackSystem] Attack finished! Released to Idle.\n";
                    continue;
                }

                if (world.transforms.has(entity)) {
                    auto& transform = world.transforms.get(entity);
                    transform.velocity = {0.0f, 0.0f};
                }

                if (attackState.hasFiredDamage) continue;
                if (!world.transforms.has(entity)) continue;

                const auto& transform = world.transforms.get(entity);
                float px = transform.position.x;
                float py = transform.position.y;
                float facingX = transform.facingX;
                float facingY = transform.facingY;

                attackState.hasFiredDamage = true;

                float halfArcRad = (attackState.attackArc / 2.0f) * (M_PI / 180.0f);
                float cosThreshold = std::cos(halfArcRad);
                float rangeSq = attackState.attackRange * attackState.attackRange;

                std::cout << "[AttackSystem] 🗡️ Sector scan START: playerPos=(" << px << "," << py
                          << ") facing=(" << facingX << "," << facingY << ")"
                          << " range=" << attackState.attackRange
                          << " arc=" << attackState.attackArc << "°"
                          << " cosThreshold=" << cosThreshold << "\n";

                auto hurtList = world.hurtboxes.entityList();
                std::cout << "[AttackSystem]   Total hurtbox entities: " << hurtList.size() << "\n";
                for (Entity victim : hurtList) {
                    if (victim == entity) continue;

                    if (world.states.has(victim) &&
                        world.states.get(victim).currentState == CharacterState::Dead) {
                        continue;
                    }

                    if (!world.transforms.has(victim)) continue;

                    const auto& vt = world.transforms.get(victim);
                    float vx = vt.position.x;
                    float vy = vt.position.y;

                    float dx = vx - px;
                    float dy = vy - py;
                    float dist = std::sqrt(dx * dx + dy * dy);

                    float hurtboxRadius = world.hurtboxes.has(victim) ? world.hurtboxes.get(victim).radius : 20.0f;

                    std::cout << "[AttackSystem]   Checking victim=" << victim
                              << " pos=(" << vx << "," << vy << ")"
                              << " dist=" << dist
                              << " (effectiveRange=" << attackState.attackRange + hurtboxRadius << ")";

                    if (dist > attackState.attackRange + hurtboxRadius) {
                        std::cout << " → TOO FAR ❌\n";
                        continue;
                    }
                    if (dist == 0.0f) continue;

                    float attackerZ = 0.0f;
                    if (world.zTransforms.has(entity)) {
                        attackerZ = world.zTransforms.get(entity).z;
                    }
                    const float attackVerticalRange = 100.0f;

                    float victimZ = 0.0f;
                    float victimHeight = 50.0f;
                    if (world.zTransforms.has(victim)) {
                        victimZ = world.zTransforms.get(victim).z;
                        victimHeight = world.zTransforms.get(victim).height;
                    } else if (world.hurtboxes.has(victim)) {
                        victimHeight = world.hurtboxes.get(victim).height;
                    }

                    bool zIntersect = (attackerZ < victimZ + victimHeight) && (attackerZ + attackVerticalRange > victimZ);
                    if (!zIntersect) {
                        std::cout << " → Z-AXIS MISS ❌\n";
                        continue;
                    }

                    float dirX = dx / dist;
                    float dirY = dy / dist;
                    float dotProduct = (facingX * dirX) + (facingY * dirY);

                    std::cout << " dot=" << dotProduct << " (threshold=" << cosThreshold << ")";

                    if (dotProduct >= cosThreshold) {
                        std::cout << " → HIT ✅\n";

                        // ← 【修复】伤害浮动 0.8x ~ 1.2x & 暴击判定（与 CollisionSystem 一致）
                        float randomMultiplier = 0.8f + (static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX)) * 0.4f;
                        bool isCritical = (randomMultiplier > 1.1f);
                        int actualDamage = static_cast<int>(attackState.baseDamage * randomMultiplier);

                        Entity eventEntity = world.ecs.create();
                        world.damageEvents.add(eventEntity, {
                            .target = victim,
                            .actualDamage = actualDamage,
                            .hitPosition = {vx, vy},
                            .isCritical = isCritical,
                            .hitDirection = {dirX, dirY},
                            .knockbackXY = 800.0f,
                            .knockbackZ = 200.0f,
                            .attacker = entity,
                            .timestamp = 0.0f
                        });
                    } else {
                        std::cout << " → OUTSIDE ARC ❌\n";
                    }
                }

                std::cout << "[AttackSystem] Sector scan complete.\n";
            } else {
                continue;
            }
        }
    }
};

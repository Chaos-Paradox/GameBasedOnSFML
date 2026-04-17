#pragma once
#include "core/GameWorld.h"
#include <cmath>
#include <iostream>
#include <unordered_map>

/**
 * @brief 炸弹系统
 *
 * ⚠️ 重构（ECS 纯净原则）：
 * - lastPosX/Y → 由系统内部 prevBombPos 映射保存
 * - isKicked → 移除，不需要
 * - hasExitedOwner → 改为纯距离判断（System 内部计算）
 * - collisionCooldown → 由系统内部 collisionCooldowns 映射维护
 * - zTrans.applyGravity() → 内联重力逻辑
 * - zTrans.isGrounded() → 内联 z <= 0 判断
 *
 * Dash 碰撞设计：
 *   1. CCD 连续碰撞检测（点到线段最短距离）
 *   2. 碰撞后用弹性碰撞公式计算双方新速度
 *   3. 碰撞冷却防止同一碰撞被重复触发
 *   4. 炸弹按弹性碰撞结果获得新速度，不再跟随玩家
 */
class BombSystem {
public:
    void update(GameWorld& world, float dt)
    {
        // 碰撞冷却递减（系统内部维护）
        auto momentumEntities = world.momentums.entityList();
        for (Entity e : momentumEntities) {
            if (world.momentums.has(e)) {
                auto& mom = world.momentums.get(e);
                auto it = collisionCooldowns.find(e);
                if (it != collisionCooldowns.end() && it->second > 0.0f) {
                    it->second -= dt;
                    if (it->second < 0.0f) it->second = 0.0f;
                }
            }
        }

        auto bombEntities = world.bombs.entityList();

        for (Entity bomb : bombEntities) {
            if (world.deathTags.has(bomb)) continue;
            if (!world.bombs.has(bomb) || !world.transforms.has(bomb) || !world.zTransforms.has(bomb)) continue;

            auto& bombComp = world.bombs.get(bomb);
            auto& transform = world.transforms.get(bomb);
            auto& zTrans = world.zTransforms.get(bomb);

            // 记录上一帧位置（系统内部保存，用于 CCD）
            prevBombPos[bomb] = {transform.position.x, transform.position.y};

            // ========== 脱离母体保护 (Exit Grace Period) ==========
            if (world.throwables.has(bomb)) {
                auto& throwComp = world.throwables.get(bomb);

                // 倒计时保护期
                if (throwComp.graceTimer > 0.0f) {
                    throwComp.graceTimer -= dt;
                }

                // 踢飞豁免期倒计时
                if (throwComp.ignoreKickerTimer > 0.0f) {
                    throwComp.ignoreKickerTimer -= dt;
                    if (throwComp.ignoreKickerTimer < 0.0f) throwComp.ignoreKickerTimer = 0.0f;
                }

                // 保护期结束后，用空间距离做长期判定（不再依赖 hasExitedOwner）
                if (throwComp.graceTimer <= 0.0f && world.transforms.has(throwComp.owner)) {
                    auto& ownerTrans = world.transforms.get(throwComp.owner);
                    float dx = transform.position.x - ownerTrans.position.x;
                    float dy = transform.position.y - ownerTrans.position.y;
                    float distSq = dx * dx + dy * dy;
                    constexpr float EXIT_THRESHOLD = 40.0f;
                    if (distSq > EXIT_THRESHOLD * EXIT_THRESHOLD) {
                        // 已脱离，不再需要标记（距离判断即可）
                        std::cout << "[Bomb] 💣 炸弹已脱离母体（空间）！dist=" << std::sqrt(distSq) << "\n";
                    }
                }
            }

            // 引信倒计时
            bombComp.fuseTimer -= dt;

            // 爆炸生成 AOE
            if (bombComp.fuseTimer <= 0.0f) {
                std::cout << "[Bomb] 💥 炸弹爆炸！\n";
                world.deathTags.add(bomb, {});

                Entity explosion = world.ecs.create();
                std::cout << "[BOMB] 💥 Created Explosion AOE with ID: " << (uint32_t)explosion << std::endl;

                world.transforms.add(explosion, {
                    .position = transform.position,
                    .scale = {1.0f, 1.0f},
                    .rotation = 0.0f,
                    .velocity = {0.0f, 0.0f},
                    .facingX = 1.0f,
                    .facingY = 0.0f
                });

                world.hitboxes.add(explosion, {
                    .radius = 150.0f,
                    .offset = {0.0f, 0.0f},
                    .damageMultiplier = 100,
                    .element = ElementType::Fire,
                    .knockbackForce = 500.0f,
                    .sourceEntity = bomb,
                    .knockbackXY = 2000.0f,
                    .knockbackZ = 800.0f,
                    .active = true
                });

                world.zTransforms.add(explosion, {
                    .z = 0.0f,
                    .vz = 0.0f,
                    .gravity = 0.0f,
                    .height = 500.0f
                });

                world.lifetimes.add(explosion, {
                    .timeLeft = 0.1f
                });

                continue;
            }

            // 原地弹跳物理（内联重力逻辑）
            // 替代 zTrans.applyGravity(dt)
            if (zTrans.z > 0.0f || zTrans.vz > 0.0f) {
                zTrans.vz += zTrans.gravity * dt;
                zTrans.z += zTrans.vz * dt;
                if (zTrans.z <= 0.0f) {
                    zTrans.z = 0.0f;
                    zTrans.vz = 0.0f;
                }
            }

            // 落地弹跳（内联 isGrounded 判断：z <= 0）
            if (zTrans.z <= 0.0f && zTrans.vz < 0.0f) {
                zTrans.z = 0.0f;
                if (std::abs(zTrans.vz) > 50.0f) {
                    zTrans.vz = -zTrans.vz * 0.5f;
                    std::cout << "[Bomb] 弹跳！vz=" << zTrans.vz << "\n";
                } else {
                    zTrans.vz = 0.0f;
                }
                transform.velocity.x *= 0.8f;
                transform.velocity.y *= 0.8f;
            }

            // ========== Dash 碰撞踢飞（CCD + 弹性碰撞） ==========
            auto charEntities = world.characters.entityList();
            for (Entity player : charEntities) {
                if (!world.states.has(player) || !world.transforms.has(player) || !world.zTransforms.has(player)) continue;

                const auto& playerState = world.states.get(player);
                auto& playerTrans = world.transforms.get(player);
                const auto& playerZTrans = world.zTransforms.get(player);

                if (playerState.currentState != CharacterState::Dash) continue;

                bool playerHasMomentum = world.momentums.has(player);
                bool bombHasMomentum = world.momentums.has(bomb);

                // 碰撞冷却检查（系统内部映射）
                auto cdIt = collisionCooldowns.find(bomb);
                if (cdIt != collisionCooldowns.end() && cdIt->second > 0.0f) continue;

                // --- 计算距离 ---
                float proximityDx = playerTrans.position.x - transform.position.x;
                float proximityDy = playerTrans.position.y - transform.position.y;
                float proximityDistance = std::sqrt(proximityDx * proximityDx + proximityDy * proximityDy);

                // --- 自碰撞拦截 (Exit Grace Period, 方向性) ---
                if (world.throwables.has(bomb)) {
                    const auto& throwComp = world.throwables.get(bomb);
                    bool inGracePeriod = throwComp.graceTimer > 0.0f;
                    bool notExited = false;
                    if (inGracePeriod) {
                        notExited = true;
                    } else if (player == throwComp.owner && world.transforms.has(throwComp.owner)) {
                        auto& ownerTrans = world.transforms.get(throwComp.owner);
                        float ddx = transform.position.x - ownerTrans.position.x;
                        float ddy = transform.position.y - ownerTrans.position.y;
                        constexpr float EXIT_THRESHOLD = 40.0f;
                        if (ddx * ddx + ddy * ddy <= EXIT_THRESHOLD * EXIT_THRESHOLD) {
                            notExited = true;
                        }
                    }
                    if (player == throwComp.owner && inGracePeriod) {
                        float dotProx = proximityDx * playerTrans.velocity.x +
                                        proximityDy * playerTrans.velocity.y;
                        if (dotProx > 0.0f) continue;
                    }
                }

                bool shouldKick = false;
                float distX = 0.0f, distY = 0.0f;

                // --- 直接距离检测 ---
                if (proximityDistance < 60.0f) {
                    float dotProx = proximityDx * playerTrans.velocity.x +
                                    proximityDy * playerTrans.velocity.y;
                    if (dotProx < 0.0f) {
                        shouldKick = true;
                        distX = proximityDx;
                        distY = proximityDy;
                    }
                }

                // --- CCD 连续碰撞检测 ---
                if (!shouldKick) {
                    float prevPlayerX = playerTrans.position.x - playerTrans.velocity.x * dt;
                    float prevPlayerY = playerTrans.position.y - playerTrans.velocity.y * dt;
                    float trajX = playerTrans.position.x - prevPlayerX;
                    float trajY = playerTrans.position.y - prevPlayerY;
                    float trajLenSq = trajX * trajX + trajY * trajY;

                    if (trajLenSq < 0.001f) {
                        distX = playerTrans.position.x - transform.position.x;
                        distY = playerTrans.position.y - transform.position.y;
                    } else {
                        float t = ((transform.position.x - prevPlayerX) * trajX +
                                   (transform.position.y - prevPlayerY) * trajY) / trajLenSq;
                        t = t < 0.0f ? 0.0f : (t > 1.0f ? 1.0f : t);
                        float closestX = prevPlayerX + t * trajX;
                        float closestY = prevPlayerY + t * trajY;
                        distX = closestX - transform.position.x;
                        distY = closestY - transform.position.y;
                    }

                    float distance = std::sqrt(distX * distX + distY * distY);
                    if (distance >= 60.0f) continue;

                    shouldKick = true;
                }

                float playerBottom = playerZTrans.z;
                float playerTop = playerZTrans.z + playerZTrans.height;
                float bombBottom = zTrans.z;
                float bombTop = zTrans.z + zTrans.height;

                bool zIntersect = !(playerBottom > bombTop || playerTop < bombBottom);
                if (!zIntersect) continue;

                // --- 碰撞法线 ---
                float nLen = std::sqrt(distX * distX + distY * distY);
                float nx, ny;
                bool usedFallback = false;
                if (nLen > 0.001f) {
                    nx = distX / nLen;
                    ny = distY / nLen;
                } else {
                    usedFallback = true;
                    if (world.dashes.has(player)) {
                        const auto& dashComp = world.dashes.get(player);
                        nx = dashComp.dashDir.x;
                        ny = dashComp.dashDir.y;
                    } else {
                        nx = playerTrans.facingX;
                        ny = playerTrans.facingY;
                    }
                    float fLen = std::sqrt(nx * nx + ny * ny);
                    if (fLen > 0.0f) { nx /= fLen; ny /= fLen; }
                    else { nx = 1.0f; ny = 0.0f; }
                }

                std::cout << "[BombKick] playerPos=" << playerTrans.position.x <<
                          " bombPos=" << transform.position.x <<
                          " proximityDist=" << proximityDistance <<
                          " nLen=" << nLen <<
                          " fallback=" << usedFallback <<
                          " nx=" << nx <<
                          " dashDir.x=" << (world.dashes.has(player) ? world.dashes.get(player).dashDir.x : 0) << "\n";

                // --- 获取质量 ---
                float playerMass = 100.0f;
                float bombMass = 10.0f;

                if (world.momentums.has(player)) {
                    playerMass = world.momentums.get(player).mass;
                }
                if (world.momentums.has(bomb)) {
                    bombMass = world.momentums.get(bomb).mass;
                }

                // --- 获取碰撞用速度 ---
                Vec2 v1 = playerTrans.velocity;
                Vec2 v2 = transform.velocity;

                bool hasDash = world.dashes.has(player);
                bool isIframe = hasDash && world.dashes.get(player).isInvincible;

                if (isIframe) {
                    const auto& dashComp = world.dashes.get(player);
                    v1 = dashComp.dashDir * dashComp.dashSpeed;
                }

                // 探针：乌龙球检测
                if (world.throwables.has(bomb)) {
                    const auto& throwComp = world.throwables.get(bomb);
                    bool inGracePeriod = throwComp.graceTimer > 0.0f;
                    bool notExited = inGracePeriod;
                    if (!inGracePeriod && player == throwComp.owner && world.transforms.has(throwComp.owner)) {
                        auto& ownerTrans = world.transforms.get(throwComp.owner);
                        float ddx = transform.position.x - ownerTrans.position.x;
                        float ddy = transform.position.y - ownerTrans.position.y;
                        constexpr float EXIT_THRESHOLD = 40.0f;
                        if (ddx * ddx + ddy * ddy <= EXIT_THRESHOLD * EXIT_THRESHOLD) {
                            notExited = true;
                        }
                    }
                    if (player == throwComp.owner) {
                        std::cout << "💥 [乌龙球] 自碰撞！graceTimer=" << throwComp.graceTimer
                                  << " playerVel=(" << playerTrans.velocity.x << ", " << playerTrans.velocity.y << ")\n";
                    }
                }

                // ========== 踢飞豁免期 ==========
                if (world.throwables.has(bomb)) {
                    auto& throwComp = world.throwables.get(bomb);
                    throwComp.lastKickedBy = player;
                    throwComp.ignoreKickerTimer = 0.15f;
                    std::cout << "[Bomb] 🔒 刚体豁免期开启，ignoreKickerTimer=0.15s\n";
                }

                // --- 弹性碰撞 ---
                float v1n = v1.x * nx + v1.y * ny;
                float v2n = v2.x * nx + v2.y * ny;

                float v1tx = v1.x - v1n * nx;
                float v1ty = v1.y - v1n * ny;
                float v2tx = v2.x - v2n * nx;
                float v2ty = v2.y - v2n * ny;

                float totalMass = playerMass + bombMass;
                float v1n_new = ((playerMass - bombMass) * v1n + 2.0f * bombMass * v2n) / totalMass;
                float v2n_new = ((bombMass - playerMass) * v2n + 2.0f * playerMass * v1n) / totalMass;

                float v1x_new = v1tx + v1n_new * nx;
                float v1y_new = v1ty + v1n_new * ny;
                float v2x_new = v2tx + v2n_new * nx;
                float v2y_new = v2ty + v2n_new * ny;

                transform.velocity.x = v2x_new;
                transform.velocity.y = v2y_new;

                playerTrans.velocity.x = v1x_new;
                playerTrans.velocity.y = v1y_new;

                if (playerHasMomentum) {
                    world.momentums.get(player).velocity = {v1x_new, v1y_new};
                }
                if (bombHasMomentum) {
                    world.momentums.get(bomb).velocity = {v2x_new, v2y_new};
                }

                // 碰撞冷却（系统内部）
                collisionCooldowns[bomb] = 0.1f;

                // --- Z 轴踢飞 ---
                float playerZ = playerZTrans.z;
                float playerVZ = playerZTrans.vz;

                if (playerZ <= 5.0f) {
                    zTrans.vz = 250.0f;
                    zTrans.z += 5.0f;
                    std::cout << "[Bomb] 地面平踢！vz=250\n";
                } else if (playerVZ > 0.0f) {
                    zTrans.vz = 450.0f;
                    std::cout << "[Bomb] 跃起挑踢！vz=450\n";
                } else {
                    zTrans.vz = 50.0f;
                    std::cout << "[Bomb] 下落扣杀！vz=50\n";
                }

                std::cout << "[BombKick] RESULT: bombVel=" << v2x_new <<
                          " playerVel=" << v1x_new <<
                          " v1n=" << v1n <<
                          " v2n_new=" << v2n_new << "\n";

                break;
            }

            // ========== 炸弹碰撞玩家（反向检测） ==========
            for (Entity player : charEntities) {
                if (!world.states.has(player) || !world.transforms.has(player) || !world.zTransforms.has(player)) continue;

                const auto& playerState = world.states.get(player);
                if (playerState.currentState == CharacterState::Dash) continue;

                auto& playerTrans = world.transforms.get(player);
                const auto& playerZTrans = world.zTransforms.get(player);

                bool playerHasMomentum = world.momentums.has(player);
                bool bombHasMomentum = world.momentums.has(bomb);

                auto cdIt = collisionCooldowns.find(bomb);
                if (cdIt != collisionCooldowns.end() && cdIt->second > 0.0f) continue;

                float bombSpeed = std::sqrt(transform.velocity.x * transform.velocity.x +
                                            transform.velocity.y * transform.velocity.y);
                constexpr float minBombSpeed = 100.0f;
                if (bombSpeed < minBombSpeed) continue;

                float dx = transform.position.x - playerTrans.position.x;
                float dy = transform.position.y - playerTrans.position.y;
                float distance = std::sqrt(dx * dx + dy * dy);

                constexpr float collisionRadius = 60.0f;
                if (distance >= collisionRadius) continue;

                float playerBottom = playerZTrans.z;
                float playerTop = playerZTrans.z + playerZTrans.height;
                float bombBottom = zTrans.z;
                float bombTop = zTrans.z + zTrans.height;

                bool zIntersect = !(playerBottom > bombTop || playerTop < bombBottom);
                if (!zIntersect) continue;

                float nx = dx;
                float ny = dy;
                float nLen = std::sqrt(nx * nx + ny * ny);
                if (nLen < 0.001f) {
                    nx = playerTrans.facingX;
                    ny = playerTrans.facingY;
                    nLen = std::sqrt(nx * nx + ny * ny);
                    if (nLen > 0.0f) { nx /= nLen; ny /= nLen; }
                    else { nx = 1.0f; ny = 0.0f; }
                } else {
                    nx /= nLen;
                    ny /= nLen;
                }

                float playerMass = 100.0f;
                float bombMass = 10.0f;

                if (world.momentums.has(player)) {
                    playerMass = world.momentums.get(player).mass;
                }
                if (world.momentums.has(bomb)) {
                    bombMass = world.momentums.get(bomb).mass;
                }

                Vec2 v1 = playerTrans.velocity;
                Vec2 v2 = transform.velocity;

                float v1n = v1.x * nx + v1.y * ny;
                float v2n = v2.x * nx + v2.y * ny;

                float v1tx = v1.x - v1n * nx;
                float v1ty = v1.y - v1n * ny;
                float v2tx = v2.x - v2n * nx;
                float v2ty = v2.y - v2n * ny;

                float totalMass = playerMass + bombMass;
                float v1n_new = ((playerMass - bombMass) * v1n + 2.0f * bombMass * v2n) / totalMass;
                float v2n_new = ((bombMass - playerMass) * v2n + 2.0f * playerMass * v1n) / totalMass;

                float v1x_new = v1tx + v1n_new * nx;
                float v1y_new = v1ty + v1n_new * ny;
                float v2x_new = v2tx + v2n_new * nx;
                float v2y_new = v2ty + v2n_new * ny;

                transform.velocity.x = v2x_new;
                transform.velocity.y = v2y_new;

                playerTrans.velocity.x = v1x_new;
                playerTrans.velocity.y = v1y_new;

                if (playerHasMomentum) {
                    world.momentums.get(player).velocity = {v1x_new, v1y_new};
                }
                if (bombHasMomentum) {
                    world.momentums.get(bomb).velocity = {v2x_new, v2y_new};
                }

                collisionCooldowns[bomb] = 0.1f;

                std::cout << "[Bomb] 💣 炸弹碰撞玩家！p_mass=" << playerMass
                          << " b_mass=" << bombMass
                          << " b_speed=" << bombSpeed
                          << " p_vel=(" << v1x_new << ", " << v1y_new << ")"
                          << " b_vel=(" << v2x_new << ", " << v2y_new << ")\n";

                break;
            }
        }
    }

private:
    // 系统内部状态
    std::unordered_map<Entity, float> collisionCooldowns;
    std::unordered_map<Entity, Vec2> prevBombPos;
};

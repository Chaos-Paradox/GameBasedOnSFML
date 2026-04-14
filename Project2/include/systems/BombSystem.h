#pragma once
#include "core/GameWorld.h"
#include <cmath>
#include <iostream>

/**
 * @brief 炸弹系统
 *
 * Dash 碰撞设计：
 *   1. CCD 连续碰撞检测（点到线段最短距离）
 *   2. 碰撞后用弹性碰撞公式计算双方新速度
 *   3. 碰撞冷却防止同一碰撞被重复触发
 *   4. 炸弹按弹性碰撞结果获得新速度，不再跟随玩家
 *
 *   1. 计算碰撞法线 n（从碰撞点指向玩家中心）
 *   2. 将速度分解为法线分量(v·n)和切线分量(v - (v·n)*n)
 *   3. 仅对法线分量应用弹性碰撞公式
 *   4. 切线分量保持不变，重新组合得到新速度
 */
class BombSystem {
public:
    void update(GameWorld& world, float dt)
    {
        // 碰撞冷却递减
        auto momentumEntities = world.momentums.entityList();
        for (Entity e : momentumEntities) {
            if (world.momentums.has(e)) {
                auto& mom = world.momentums.get(e);
                if (mom.collisionCooldown > 0.0f) {
                    mom.collisionCooldown -= dt;
                    if (mom.collisionCooldown < 0.0f) mom.collisionCooldown = 0.0f;
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

            // 记录上一帧位置（用于下一帧的 CCD 连续碰撞检测）
            bombComp.lastPosX = transform.position.x;
            bombComp.lastPosY = transform.position.y;

            // 重置踢飞标记（每帧刷新）
            bombComp.isKicked = false;

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
                    .timeLeft = 0.1f,
                    .autoDestroy = true
                });

                continue;
            }

            // 原地弹跳物理
            zTrans.applyGravity(dt);

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

                // 获取玩家状态和变换（transform mutable，用于修改速度）
                const auto& playerState = world.states.get(player);
                auto& playerTrans = world.transforms.get(player);
                const auto& playerZTrans = world.zTransforms.get(player);

                if (playerState.currentState != CharacterState::Dash) continue;

                // --- 碰撞冷却检查 ---
                bool playerHasMomentum = world.momentums.has(player);
                bool bombHasMomentum = world.momentums.has(bomb);

                if (bombHasMomentum && world.momentums.get(bomb).collisionCooldown > 0.0f) continue;

                // --- 直接距离检测（CCD 补充） ---
                // 玩家贴着炸弹时（< 60px，与 CCD 阈值一致），直接触发踢飞
                // 这解决了 prevPlayerX 用 velocity*dt 估算不准确的问题：
                // 当玩家与炸弹重叠或极近时，CCD 线段检测可能错过炸弹
                float proximityDx = playerTrans.position.x - transform.position.x;
                float proximityDy = playerTrans.position.y - transform.position.y;
                float proximityDistance = std::sqrt(proximityDx * proximityDx + proximityDy * proximityDy);

                bool shouldKick = false;
                float kickNx = 0.0f, kickNy = 0.0f; // 预计算的碰撞法线
                float distX = 0.0f, distY = 0.0f;   // 碰撞法线向量（两种路径都会设置）

                if (proximityDistance < 60.0f) {
                    // 贴着！直接踢飞，不依赖 CCD
                    shouldKick = true;
                    distX = proximityDx;
                    distY = proximityDy;

                    // 贴脸踢飞：用 dashDir（实际冲刺方向）而非 facing
                    // facing 可能和 dash 方向不一致（如面向右但向左冲刺）
                    if (world.dashes.has(player)) {
                        const auto& dashComp = world.dashes.get(player);
                        kickNx = dashComp.dashDir.x;
                        kickNy = dashComp.dashDir.y;
                    } else {
                        kickNx = playerTrans.facingX;
                        kickNy = playerTrans.facingY;
                    }
                    float fLen = std::sqrt(kickNx * kickNx + kickNy * kickNy);
                    if (fLen > 0.0f) { kickNx /= fLen; kickNy /= fLen; }
                    else { kickNx = 1.0f; kickNy = 0.0f; }

                    std::cout << "[Bomb] 直接距离检测触发踢飞！dist=" << proximityDistance
                              << " kickDir=(" << kickNx << "," << kickNy << ")\n";
                }

                // --- CCD 连续碰撞检测（用于远距离踢飞） ---
                if (!shouldKick) {
                    // 由于 BombSystem 在 MovementSystem 之前运行，玩家 position 还未被更新。
                    // 对于新创建的炸弹（velocity=0），bomb 的 lastPos→currentPos 是零长度线段，
                    // 导致回退到点检测，使用玩家当前位置（冲刺前），无法检测到远距离冲刺。
                    // 解决：用玩家的冲刺轨迹（prevPos → currentPos）与炸弹位置做最近点检测。
                    float prevPlayerX = playerTrans.position.x - playerTrans.velocity.x * dt;
                    float prevPlayerY = playerTrans.position.y - playerTrans.velocity.y * dt;
                    float trajX = playerTrans.position.x - prevPlayerX;
                    float trajY = playerTrans.position.y - prevPlayerY;
                    float trajLenSq = trajX * trajX + trajY * trajY;

                    if (trajLenSq < 0.001f) {
                        // 玩家几乎没有移动，直接检查距离
                        distX = playerTrans.position.x - transform.position.x;
                        distY = playerTrans.position.y - transform.position.y;
                    } else {
                        float t = ((transform.position.x - prevPlayerX) * trajX +
                                   (transform.position.y - prevPlayerY) * trajY) / trajLenSq;
                        t = t < 0.0f ? 0.0f : (t > 1.0f ? 1.0f : t);
                        float closestX = prevPlayerX + t * trajX;
                        float closestY = prevPlayerY + t * trajY;
                        distX = transform.position.x - closestX;
                        distY = transform.position.y - closestY;
                    }

                    float distance = std::sqrt(distX * distX + distY * distY);
                    if (distance >= 60.0f) continue;

                    shouldKick = true;
                    // CCD 检测：用 dashDir（实际冲刺方向）而非 facing
                    if (world.dashes.has(player)) {
                        const auto& dashComp = world.dashes.get(player);
                        kickNx = dashComp.dashDir.x;
                        kickNy = dashComp.dashDir.y;
                    } else {
                        kickNx = playerTrans.facingX;
                        kickNy = playerTrans.facingY;
                    }
                    float fLen = std::sqrt(kickNx * kickNx + kickNy * kickNy);
                    if (fLen > 0.0f) { kickNx /= fLen; kickNy /= fLen; }
                    else { kickNx = 1.0f; kickNy = 0.0f; }
                }

                float playerBottom = playerZTrans.z;
                float playerTop = playerZTrans.z + playerZTrans.height;
                float bombBottom = zTrans.z;
                float bombTop = zTrans.z + zTrans.height;

                bool zIntersect = !(playerBottom > bombTop || playerTop < bombBottom);
                if (!zIntersect) continue;

                // --- 碰撞法线：直接用之前计算的 kickNx/kickNy ---
                // 炸弹被踢向玩家冲刺方向（facing），不受穿模影响
                float nx = kickNx;
                float ny = kickNy;

                // --- 获取质量（从 MomentumComponent 读取，单一数据源） ---
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

                // 无敌帧检测：用 dashSpeed 作为玩家碰撞速度
                bool hasDash = world.dashes.has(player);
                bool isIframe = hasDash && world.dashes.get(player).isInvincible;

                if (isIframe) {
                    const auto& dashComp = world.dashes.get(player);
                    v1 = dashComp.dashDir * dashComp.dashSpeed;
                }

                bombComp.isKicked = true;

                // --- 弹性碰撞（沿法线方向） ---
                // 1. 将速度分解为法线分量和切线分量
                float v1n = v1.x * nx + v1.y * ny;  // v1 · n
                float v2n = v2.x * nx + v2.y * ny;  // v2 · n

                // 切线分量 = 原速度 - 法线分量
                float v1tx = v1.x - v1n * nx;
                float v1ty = v1.y - v1n * ny;
                float v2tx = v2.x - v2n * nx;
                float v2ty = v2.y - v2n * ny;

                // 2. 仅对法线分量应用弹性碰撞公式
                float totalMass = playerMass + bombMass;
                float v1n_new = ((playerMass - bombMass) * v1n + 2.0f * bombMass * v2n) / totalMass;
                float v2n_new = ((bombMass - playerMass) * v2n + 2.0f * playerMass * v1n) / totalMass;

                // 3. 新速度 = 切线分量 + 新法线分量
                float v1x_new = v1tx + v1n_new * nx;
                float v1y_new = v1ty + v1n_new * ny;
                float v2x_new = v2tx + v2n_new * nx;
                float v2y_new = v2ty + v2n_new * ny;

                // --- 应用结果 ---
                transform.velocity.x = v2x_new;
                transform.velocity.y = v2y_new;

                playerTrans.velocity.x = v1x_new;
                playerTrans.velocity.y = v1y_new;

                // 同步 momentum 速度
                if (playerHasMomentum) {
                    world.momentums.get(player).velocity = {v1x_new, v1y_new};
                }
                if (bombHasMomentum) {
                    world.momentums.get(bomb).velocity = {v2x_new, v2y_new};
                }

                // --- 碰撞冷却（仅炸弹） ---
                if (bombHasMomentum) {
                    world.momentums.get(bomb).collisionCooldown = 0.1f;
                } else {
                    world.momentums.add(bomb, {
                        .velocity = {v2x_new, v2y_new},
                        .collisionCooldown = 0.1f
                    });
                }

                // --- Z 轴踢飞（根据玩家位置决定） ---
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

                std::cout << "[Bomb] ⚽ 弹性碰撞击飞！p_mass=" << playerMass
                          << " b_mass=" << bombMass
                          << " p_vel=(" << v1x_new << ", " << v1y_new << ")"
                          << " b_vel=(" << v2x_new << ", " << v2y_new << ")\n";

                // 每个炸弹每帧只允许被一个玩家踢一次
                break;
            }

            // ========== 炸弹碰撞玩家（反向检测：炸弹飞行中撞到玩家） ==========
            // 当炸弹速度足够快时，即使玩家没有 dash，也触发弹性碰撞
            for (Entity player : charEntities) {
                if (!world.states.has(player) || !world.transforms.has(player) || !world.zTransforms.has(player)) continue;

                const auto& playerState = world.states.get(player);
                if (playerState.currentState == CharacterState::Dash) continue; // dash 碰撞已由上方处理

                auto& playerTrans = world.transforms.get(player);
                const auto& playerZTrans = world.zTransforms.get(player);

                // --- 碰撞冷却检查 ---
                bool playerHasMomentum = world.momentums.has(player);
                bool bombHasMomentum = world.momentums.has(bomb);

                if (bombHasMomentum && world.momentums.get(bomb).collisionCooldown > 0.0f) continue;

                // --- 检查炸弹是否足够快（飞行中） ---
                float bombSpeed = std::sqrt(transform.velocity.x * transform.velocity.x +
                                            transform.velocity.y * transform.velocity.y);
                constexpr float minBombSpeed = 100.0f; // 低于此速度不触发碰撞（已落地的炸弹）
                if (bombSpeed < minBombSpeed) continue;

                // --- AABB 距离检测 ---
                float dx = transform.position.x - playerTrans.position.x;
                float dy = transform.position.y - playerTrans.position.y;
                float distance = std::sqrt(dx * dx + dy * dy);

                constexpr float collisionRadius = 60.0f; // 碰撞判定半径
                if (distance >= collisionRadius) continue;

                // --- Z 轴高度重叠检查 ---
                float playerBottom = playerZTrans.z;
                float playerTop = playerZTrans.z + playerZTrans.height;
                float bombBottom = zTrans.z;
                float bombTop = zTrans.z + zTrans.height;

                bool zIntersect = !(playerBottom > bombTop || playerTop < bombBottom);
                if (!zIntersect) continue;

                // --- 碰撞法线：从炸弹指向玩家 ---
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

                // --- 获取质量（从 MomentumComponent 读取，单一数据源） ---
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

                // --- 弹性碰撞（沿法线方向） ---
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

                // --- 应用结果 ---
                transform.velocity.x = v2x_new;
                transform.velocity.y = v2y_new;

                playerTrans.velocity.x = v1x_new;
                playerTrans.velocity.y = v1y_new;

                // 同步 momentum 速度
                if (playerHasMomentum) {
                    world.momentums.get(player).velocity = {v1x_new, v1y_new};
                }
                if (bombHasMomentum) {
                    world.momentums.get(bomb).velocity = {v2x_new, v2y_new};
                }

                // --- 碰撞冷却 ---
                if (bombHasMomentum) {
                    world.momentums.get(bomb).collisionCooldown = 0.1f;
                } else {
                    world.momentums.add(bomb, {
                        .velocity = {v2x_new, v2y_new},
                        .collisionCooldown = 0.1f
                    });
                }

                // 标记为被踢飞
                bombComp.isKicked = true;

                std::cout << "[Bomb] 💣 炸弹碰撞玩家！p_mass=" << playerMass
                          << " b_mass=" << bombMass
                          << " b_speed=" << bombSpeed
                          << " p_vel=(" << v1x_new << ", " << v1y_new << ")"
                          << " b_vel=(" << v2x_new << ", " << v2y_new << ")\n";

                // 每个炸弹每帧只允许碰撞一个玩家
                break;
            }

        }
    }

private:
    void updateMomentumVelocity(GameWorld& world, Entity entity, Vec2 newVel)
    {
        if (world.momentums.has(entity)) {
            world.momentums.get(entity).velocity = newVel;
        }
    }
};

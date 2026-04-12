#pragma once
#include "core/GameWorld.h"
#include <cmath>
#include <iostream>

/**
 * @brief 炸弹系统
 */
class BombSystem {
public:
    void update(GameWorld& world, float dt)
    {
        auto bombEntities = world.bombs.entityList();

        for (Entity bomb : bombEntities) {
            if (world.deathTags.has(bomb)) continue;
            if (!world.bombs.has(bomb) || !world.transforms.has(bomb) || !world.zTransforms.has(bomb)) continue;

            auto& bombComp = world.bombs.get(bomb);
            auto& transform = world.transforms.get(bomb);
            auto& zTrans = world.zTransforms.get(bomb);


            // 重置踢飞标记（每帧刷新，允许被任何 Dash 玩家反复踢）
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

            // Dash 碰撞踢飞检测（CCD 连续碰撞检测）
            // 如果上一帧已被踢飞，跳过检测（防止被同一/其他玩家反复踢）
            if (bombComp.isKicked) goto skipKick;
            bombComp.isKicked = false; // 重置标记，允许本帧检测
            auto charEntities = world.characters.entityList();
            for (Entity player : charEntities) {
                if (!world.states.has(player) || !world.transforms.has(player) || !world.zTransforms.has(player)) continue;

                const auto& playerState = world.states.get(player);
                const auto& playerTrans = world.transforms.get(player);
                const auto& playerZTrans = world.zTransforms.get(player);

                if (playerState.currentState != CharacterState::Dash) continue;

                // 点到线段最短距离（CCD）
                float startX = bombComp.lastPosX;
                float startY = bombComp.lastPosY;
                float endX = transform.position.x;
                float endY = transform.position.y;
                float lineVecX = endX - startX;
                float lineVecY = endY - startY;
                float lineLenSq = lineVecX * lineVecX + lineVecY * lineVecY;

                float distX, distY;
                if (lineLenSq < 0.001f) {
                    // 炸弹几乎未移动，退化为点-点检测
                    distX = playerTrans.position.x - endX;
                    distY = playerTrans.position.y - endY;
                } else {
                    float t = ((playerTrans.position.x - startX) * lineVecX +
                               (playerTrans.position.y - startY) * lineVecY) / lineLenSq;
                    t = t < 0.0f ? 0.0f : (t > 1.0f ? 1.0f : t);
                    float closestX = startX + t * lineVecX;
                    float closestY = startY + t * lineVecY;
                    distX = playerTrans.position.x - closestX;
                    distY = playerTrans.position.y - closestY;
                }

                float distance = std::sqrt(distX * distX + distY * distY);
                if (distance >= 60.0f) continue;

                float playerBottom = playerZTrans.z;
                float playerTop = playerZTrans.z + playerZTrans.height;
                float bombBottom = zTrans.z;
                float bombTop = zTrans.z + zTrans.height;

                bool zIntersect = !(playerBottom > bombTop || playerTop < bombBottom);
                if (!zIntersect) continue;

                float facingX = playerTrans.facingX;
                float facingY = playerTrans.facingY;
                float len = std::sqrt(facingX * facingX + facingY * facingY);
                if (len > 0.0f) {
                    facingX /= len;
                    facingY /= len;
                } else {
                    facingX = 1.0f;
                    facingY = 0.0f;
                }

                bombComp.isKicked = true;

                float kickPower = 1500.0f;
                transform.velocity.x = facingX * kickPower;
                transform.velocity.y = facingY * kickPower;

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
                    transform.velocity.x *= 1.3f;
                    transform.velocity.y *= 1.3f;
                    std::cout << "[Bomb] 下落扣杀！vz=50, 水平速度×1.3\n";
                }

                std::cout << "[Bomb] ⚽ 被踢飞！velocity=(" << transform.velocity.x
                          << ", " << transform.velocity.y << ")\n";
            }

        }
    }
};

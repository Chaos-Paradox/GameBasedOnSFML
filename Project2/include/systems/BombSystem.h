#pragma once
#include "../core/Component.h"
#include "../core/ECS.h"
#include "../components/BombComponent.h"
#include "../components/Transform.h"
#include "../components/ZTransformComponent.h"
#include "../components/StateMachine.h"
#include "../components/Character.h"
#include "../components/Hitbox.h"
#include "../components/Lifetime.h"
#include "../components/DeathTag.h"
#include <cmath>
#include <iostream>

/**
 * @brief 炸弹系统
 * 
 * ⚠️ 架构设计：
 * - 引信倒计时（3 秒爆炸）
 * - 原地弹跳物理（依赖 ZTransformComponent）
 * - Dash 碰撞踢飞（动量传递）
 * - 爆炸 AOE 生成
 * - DeathTag 延迟销毁（防止崩溃）
 * 
 * @see BombComponent - 炸弹组件
 * @see DeathTag - 死亡标记
 */
class BombSystem {
public:
    void update(
        ComponentStore<BombComponent>& bombs,
        ComponentStore<TransformComponent>& transforms,
        ComponentStore<ZTransformComponent>& zTransforms,
        const ComponentStore<StateMachineComponent>& states,
        const ComponentStore<CharacterComponent>& characters,
        ComponentStore<HitboxComponent>& hitboxes,
        ComponentStore<LifetimeComponent>& lifetimes,
        ComponentStore<TransformComponent>& hitboxTransforms,
        ComponentStore<DeathTag>& deathTags,  // ← 新增：DeathTag
        ECS& ecs,
        float dt)
    {
        auto bombEntities = bombs.entityList();
        
        for (Entity bomb : bombEntities) {
            // ========== 0. 状态保护：已销毁的实体跳过 ==========
            if (deathTags.has(bomb)) continue;
            
            if (!bombs.has(bomb) || !transforms.has(bomb) || !zTransforms.has(bomb)) {
                continue;
            }
            
            auto& bombComp = bombs.get(bomb);
            auto& transform = transforms.get(bomb);
            auto& zTrans = zTransforms.get(bomb);
            
            // ========== 1. 引信倒计时 ==========
            bombComp.fuseTimer -= dt;
            
            // ========== 2. 爆炸生成 AOE（原子化操作） ==========
            if (bombComp.fuseTimer <= 0.0f) {
                std::cout << "[Bomb] 💥 炸弹爆炸！\n";
                
                // ← 【核心修复】第一时间标记 DeathTag，防止重复引爆
                deathTags.add(bomb, {});
                
                // 生成爆炸 Hitbox（巨大圆形 AOE）
                Entity explosion = ecs.create();
                
                hitboxTransforms.add(explosion, {
                    .position = transform.position,
                    .scale = {1.0f, 1.0f},
                    .rotation = 0.0f,
                    .velocity = {0.0f, 0.0f}
                });
                
                hitboxes.add(explosion, {
                    .radius = 150.0f,           // 巨大爆炸半径
                    .offset = {0.0f, 0.0f},     // 无偏移
                    .damageMultiplier = 100,    // 爆炸伤害
                    .element = ElementType::Fire,
                    .knockbackForce = 500.0f,
                    .sourceEntity = bomb,       // 炸弹是攻击源
                    .hitHistory = {},
                    .hitCount = 0,
                    .active = true
                });
                
                // Z 轴覆盖天地（从 0 到 500 都能炸到）
                zTransforms.add(explosion, {
                    .z = 0.0f,
                    .vz = 0.0f,
                    .gravity = 0.0f,
                    .height = 500.0f  // 覆盖所有高度
                });
                
                // 存在 0.1 秒后销毁
                lifetimes.add(explosion, {
                    .timeLeft = 0.1f,
                    .autoDestroy = true
                });
                
                // ← 【核心修复】不再直接 ecs.destroy，而是依赖 CleanupSystem
                // ecs.destroy(bomb);  ❌ 错误！会导致崩溃
                // continue;  // 也不需要了，因为 DeathTag 会在下一帧跳过
                
                continue;  // 跳过后续逻辑
            }
            
            // ========== 3. 原地弹跳物理 ==========
            // 应用重力
            zTrans.applyGravity(dt);
            
            // 落地检测与反弹
            if (zTrans.z <= 0.0f && zTrans.vz < 0.0f) {
                zTrans.z = 0.0f;
                
                // 如果下落速度大于阈值，则反弹（高度衰减）
                if (std::abs(zTrans.vz) > 50.0f) {
                    zTrans.vz = -zTrans.vz * 0.5f;  // 反弹并衰减
                    std::cout << "[Bomb] 弹跳！vz=" << zTrans.vz << "\n";
                } else {
                    zTrans.vz = 0.0f;  // 停稳
                }
                
                // 落地时产生强大的 XY 摩擦力，让其极快停下
                transform.velocity.x *= 0.8f;
                transform.velocity.y *= 0.8f;
            }
            
            // ========== 4. Dash 碰撞踢飞检测 ==========
            // 遍历所有玩家（通过 CharacterComponent）
            auto charEntities = characters.entityList();
            for (Entity player : charEntities) {
                if (!states.has(player) || !transforms.has(player) || !zTransforms.has(player)) {
                    continue;
                }
                
                const auto& playerState = states.get(player);
                const auto& playerTrans = transforms.get(player);
                const auto& playerZTrans = zTransforms.get(player);
                
                // 只在 Dash 状态下检测
                if (playerState.currentState != CharacterState::Dash) {
                    continue;
                }
                
                // 计算 XY 距离
                float dx = playerTrans.position.x - transform.position.x;
                float dy = playerTrans.position.y - transform.position.y;
                float distance = std::sqrt(dx * dx + dy * dy);
                
                // 碰撞检测（距离 < 40px）
                if (distance >= 40.0f) {
                    continue;
                }
                
                // Z 轴相交检测
                float playerBottom = playerZTrans.z;
                float playerTop = playerZTrans.z + playerZTrans.height;
                float bombBottom = zTrans.z;
                float bombTop = zTrans.z + zTrans.height;
                
                bool zIntersect = !(playerBottom > bombTop || playerTop < bombBottom);
                if (!zIntersect) {
                    continue;
                }
                
                // ========== 5. 发生踢飞！ ==========
                // 获取玩家 facing 方向
                float facingX = playerTrans.facingX;
                float facingY = playerTrans.facingY;
                
                // 归一化（防止为 0）
                float len = std::sqrt(facingX * facingX + facingY * facingY);
                if (len > 0.0f) {
                    facingX /= len;
                    facingY /= len;
                } else {
                    facingX = 1.0f;
                    facingY = 0.0f;
                }
                
                // 标记为已踢飞
                bombComp.isKicked = true;
                
                // ========== 6. 动量传递（核心物理） ==========
                // 1. XY 平面加速（炸弹比人轻，飞得快）
                float kickPower = 1500.0f;  // ← 击飞速度减半（从 3000 改为 1500）
                transform.velocity.x = facingX * kickPower;
                transform.velocity.y = facingY * kickPower;
                
                // 2. Z 轴姿态动量传递
                float playerZ = playerZTrans.z;
                float playerVZ = playerZTrans.vz;
                
                if (playerZ <= 5.0f) {
                    // A. 地面平踢：标准抛物线
                    zTrans.vz = 250.0f;  // ← 减半（从 500 改为 250）
                    zTrans.z += 5.0f;  // 强行稍微抬起防卡地
                    std::cout << "[Bomb] 地面平踢！vz=250\n";
                } else if (playerVZ > 0.0f) {
                    // B. 跃起挑踢：极高抛物线
                    zTrans.vz = 450.0f;  // ← 减半（从 900 改为 450）
                    std::cout << "[Bomb] 跃起挑踢！vz=450\n";
                } else {
                    // C. 下落扣杀 (Spike)：贴地极速直线
                    zTrans.vz = 50.0f;  // ← 减半（从 100 改为 50）
                    transform.velocity.x *= 1.3f;  // 扣杀带来更高的水平速度
                    transform.velocity.y *= 1.3f;
                    std::cout << "[Bomb] 下落扣杀！vz=50, 水平速度×1.3\n";
                }
                
                std::cout << "[Bomb] ⚽ 被踢飞！velocity=(" << transform.velocity.x 
                          << ", " << transform.velocity.y << ")\n";
                
                // 踢飞后跳出玩家循环
                break;
            }
        }
    }
};

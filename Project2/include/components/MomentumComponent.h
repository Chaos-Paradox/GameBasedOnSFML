#pragma once
#include "../math/Vec2.h"

/**
 * @brief 动量组件（纯数据，用于弹性碰撞计算）
 *
 * 设计要点：
 *   - mass: 质量（碰撞分离权重 + 弹性碰撞计算的数据源）
 *   - velocity: 用于碰撞计算的独立速度（可与 TransformComponent::velocity 不同）
 *   - collisionCooldown: 碰撞后冷却时间，防止同一帧内多次触发碰撞
 *   - prevPosX/Y: 上一帧位置（用于 CCD 连续碰撞检测）
 *   - useCCD: 是否启用连续碰撞检测（高速实体如炸弹）
 *
 * 弹性碰撞公式：
 *   v1_new = ((m1-m2)*v1 + 2*m2*v2) / (m1+m2)
 *   v2_new = ((m2-m1)*v2 + 2*m1*v1) / (m1+m2)
 *
 * @see PhysicalCollisionSystem - 物理碰撞 + CCD
 * @see BombSystem - dash 碰撞击飞
 */
struct MomentumComponent {
    float mass{1.0f};               // 质量（kg），碰撞计算的数据源
    Vec2 velocity{0.0f, 0.0f};     // 碰撞用速度
    float collisionCooldown{0.0f}; // 碰撞冷却倒计时（秒）

    // CCD 连续碰撞检测
    float prevPosX{0.0f};           // 上一帧 X 位置
    float prevPosY{0.0f};           // 上一帧 Y 位置
    bool useCCD{false};             // 是否启用 CCD（高速实体设为 true）
};

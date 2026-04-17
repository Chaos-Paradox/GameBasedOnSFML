#pragma once
#include "../math/Vec2.h"

/**
 * @brief 动量组件（纯数据，用于弹性碰撞计算）
 *
 * ⚠️ 重构（ECS 纯净原则）：
 * - collisionCooldown 已移除 → 碰撞冷却由 PhysicalCollisionSystem 内部维护
 * - prevPosX/Y 是"上一帧位置"，属于历史记录 → 由 PhysicalCollisionSystem 内部保存
 *
 * @see PhysicalCollisionSystem - 物理碰撞 + CCD
 * @see BombSystem - dash 碰撞击飞
 */
struct MomentumComponent {
    float mass{1.0f};               // 质量（kg），碰撞计算的数据源
    Vec2 velocity{0.0f, 0.0f};     // 碰撞用速度
    bool useCCD{false};             // 是否启用 CCD（高速实体设为 true）
};

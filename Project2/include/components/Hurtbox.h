#pragma once
#include "../utils/CollisionTypes.h"
#include "../math/Vec2.h"

/**
 * @brief Hurtbox（受击盒）组件 - 2.5D 圆柱体判定
 *
 * ⚠️ 重构（ECS 纯净原则）：
 * - invincibleTime 已移除 → 无敌时间由 HurtState 的 StateMachineComponent.stateTimer 表达
 *
 * @see AttackSystem - 扇形斩击 vs 圆柱体检测
 * @see VisualSandbox - 渲染层视觉修正
 */
struct HurtboxComponent {
    float radius{20.0f};               // 圆柱体底面半径（像素）
    float height{50.0f};               // 圆柱体高度（像素）
    float zOffset{0.0f};               // 底面距离真实Z轴的偏移（通常为0）
    Vec2 offset{0.0f, 0.0f};           // 相对于实体中心的局部偏移（像素）
    Faction faction{Faction::None};    // 阵营
    int layer{CollisionLayer::DEFAULT};// 碰撞层级
};

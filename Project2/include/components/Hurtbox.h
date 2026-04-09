#pragma once
#include "../utils/CollisionTypes.h"
#include "../math/Vec2.h"

/**
 * @brief Hurtbox（受击盒）组件 - 2.5D 圆柱体判定
 * 
 * ⚠️ 架构设计：
 * - 圆形判定，避免斜向攻击时矩形边角的额外距离
 * - XY 平面为圆形，Z 轴为高度（从 ZTransformComponent 读取）
 * - offset 用于局部偏移（相对于实体中心）
 * 
 * @see CollisionSystem - 2.5D 碰撞检测
 */
struct HurtboxComponent {
    float radius{20.0f};               // 圆柱体底面半径（像素）
    Vec2 offset{0.0f, 0.0f};           // 相对于实体中心的局部偏移（像素）
    Faction faction{Faction::None};    // 阵营
    int layer{CollisionLayer::DEFAULT};// 碰撞层级
    float invincibleTime{0.0f};        // 无敌剩余时间（秒）
};

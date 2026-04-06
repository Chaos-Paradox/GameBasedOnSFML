#pragma once
#include "../utils/CollisionTypes.h"

/**
 * @brief Hurtbox（受击盒）组件 - 2.5D 圆柱体判定
 * 
 * ⚠️ 架构设计：
 * - 从 Rect 改为圆形半径，支持 3D 圆柱体相交检测
 * - XY 平面为圆形，Z 轴为高度（从 ZTransformComponent 读取）
 * 
 * @see CollisionSystem - 2.5D 碰撞检测
 */
struct HurtboxComponent {
    float radius{20.0f};               // 圆柱体底面半径（像素）
    Faction faction{Faction::None};    // 阵营
    int layer{CollisionLayer::DEFAULT};// 碰撞层级
    float invincibleTime{0.0f};        // 无敌剩余时间（秒）
};

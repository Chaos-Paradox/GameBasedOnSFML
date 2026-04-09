#pragma once

/**
 * @brief 圆柱体碰撞器组件（2.5D 物理）
 * 
 * ⚠️ 架构设计：
 * - 用于实体间的物理阻挡（推挤、碰撞）
 * - 与战斗碰撞系统（Hitbox/Hurtbox）完全独立
 * - 圆柱体模型：XY 平面为圆形，Z 轴为高度
 * 
 * @see PhysicalCollisionSystem - 物理排斥系统
 */
struct ColliderComponent {
    float radius{20.0f};    // 圆柱体的底面半径（像素）
    bool isStatic{false};   // 是否是不可推动的死物（墙壁、障碍物）
    float mass{1.0f};       // 质量（用于碰撞分离权重计算）
    
    // 快速检测：是否为动态实体
    bool isDynamic() const {
        return !isStatic;
    }
};

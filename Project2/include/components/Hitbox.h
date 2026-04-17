#pragma once
#include "../math/Vec2.h"
#include "../core/Entity.h"
#include <cstdint>

enum class ElementType : uint8_t {
    Physical,
    Fire,
    Toxic,
    Ice,
};

/**
 * @brief Hitbox（攻击盒）组件 - 2.5D 圆柱体判定
 *
 * ⚠️ 架构设计：
 * - 圆形判定，避免斜向攻击时矩形边角的额外距离
 * - XY 平面为圆形，Z 轴为高度（从 ZTransformComponent 读取）
 * - offset 用于局部偏移（相对于攻击者位置）
 *
 * @see CollisionSystem - 2.5D 碰撞检测
 * @see CollisionSystem - 已打击记录由系统内部维护（不存入组件）
 */
struct HitboxComponent {
    float radius{20.0f};              // 圆柱体底面半径（像素）
    Vec2 offset{0.0f, 0.0f};          // 相对于攻击者的局部偏移（像素）
    int damageMultiplier{100};        // 伤害倍率（百分比）
    ElementType element{};            // 元素类型
    float knockbackForce{100.0f};     // 击退力（旧字段，保留兼容）
    Entity sourceEntity{INVALID_ENTITY};  // 攻击者实体 ID

    // ← 分离 XY 和 Z 轴击飞力度，支持精确控制
    float knockbackXY{0.0f};          // 水平击飞力度
    float knockbackZ{0.0f};           // 垂直挑飞力度

    bool active{false};               // 是否激活
};

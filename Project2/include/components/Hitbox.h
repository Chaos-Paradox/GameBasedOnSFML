#pragma once
#include "../core/Entity.h"
#include "../math/Vec2.h"
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
 */
struct HitboxComponent {
    float radius{20.0f};              // 圆柱体底面半径（像素）
    Vec2 offset{0.0f, 0.0f};          // 相对于攻击者的局部偏移（像素）
    int damageMultiplier{100};        // 伤害倍率（百分比）
    ElementType element{};            // 元素类型
    float knockbackForce{100.0f};     // 击退力
    Entity sourceEntity{INVALID_ENTITY};  // 攻击者实体 ID
    
    static constexpr int MAX_HIT_COUNT = 16;
    Entity hitHistory[MAX_HIT_COUNT]{};  // 命中历史（防止重复伤害）
    int hitCount{0};
    
    bool active{false};               // 是否激活
};

#pragma once
#include "../math/Vec2.h"

/**
 * @brief 变换组件（纯数据，无 SFML 依赖）
 * 
 * 存储实体的位置、旋转、缩放和速度
 * 
 * @see docs/01_DATA_SCHEMA.md - 数据字典规范
 * @see MovementSystem - 移动系统
 */
struct TransformComponent {
    Vec2 position{0.0f, 0.0f};
    Vec2 scale{1.0f, 1.0f};
    float rotation{0.0f}; // 弧度
    
    // 速度（由 MovementSystem 写入）
    Vec2 velocity{0.0f, 0.0f};
    
    // ← 新增：朝向（用于攻击判定框生成方向）
    float facingX{1.0f};
    float facingY{0.0f};
};

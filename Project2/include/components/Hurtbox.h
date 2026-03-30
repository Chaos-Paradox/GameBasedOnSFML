#pragma once
#include "../math/Rect.h"
#include "../utils/CollisionTypes.h"

/**
 * @brief Hurtbox（受击盒）组件 - 纯数据
 * 
 * ⚠️ 注意：本组件只存储数据，不包含任何逻辑
 * 
 * @see docs/01_DATA_SCHEMA.md - 数据字典规范
 */
struct HurtboxComponent {
    Rect bounds;                       ///< 受击区域（相对实体位置）
    Faction faction{Faction::None};    ///< 阵营
    int layer{CollisionLayer::DEFAULT};///< 碰撞层级
    float invincibleTime{0.0f};        ///< 无敌剩余时间（秒）
};

#pragma once
#include "../core/Entity.h"
#include "../math/Vec2.h"

/**
 * @brief 依附组件（Attatchment）
 * 
 * ⚠️ 架构设计：
 * - 用于将实体（Hitbox、特效等）绑定到主人实体上
 * - 每帧同步主人的 XY 位置和 Z 轴高度
 * - 支持相对偏移量（offset）
 * 
 * @see AttachmentSystem - 同步系统
 */
struct AttachedComponent {
    Entity owner{INVALID_ENTITY};  // 绑定的主人实体 ID
    Vec2 offset{0.0f, 0.0f};       // 相对于主人的 XY 偏移量（像素）
};

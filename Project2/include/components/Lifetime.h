#pragma once

/**
 * @brief 生命周期组件（纯数据）
 *
 * 用于临时实体（如 Hitbox）的自动销毁
 * 由 CleanupSystem 每帧读取 timeLeft，归零时标记 DeathTag
 *
 * @see CleanupSystem - 清理系统
 */
struct LifetimeComponent {
    float timeLeft{1.0f};   ///< 剩余存活时间（秒）
};

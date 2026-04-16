#pragma once
#include "../core/ECS.h"

/**
 * @brief 投掷物组件 — 标记一个实体是由某玩家投掷的
 *
 * 脱离母体保护 (Exit Grace Period)：
 *   - graceTimer: 扔出后的绝对保护时间窗口，期间无视母体碰撞
 *   - hasExitedOwner: 空间脱离后才设为 true（graceTimer 到期后的长期标记）
 *
 * 踢飞豁免期 (Collision Immunity)：
 *   - lastKickedBy: 最后一个踢它的玩家
 *   - ignoreKickerTimer: 踢飞后忽略该玩家刚体碰撞的倒计时
 */
struct ThrowableComponent {
    Entity owner{INVALID_ENTITY};        // 投掷者实体
    float graceTimer{0.15f};             // 扔出后的绝对保护时间（秒），期间无视母体碰撞
    bool hasExitedOwner{false};          // 是否已经完全飞出投掷者的碰撞盒

    // 踢飞豁免期
    Entity lastKickedBy{INVALID_ENTITY}; // 最后一个踢它的玩家
    float ignoreKickerTimer{0.0f};       // 忽略与该玩家刚体碰撞的倒计时（秒）
};

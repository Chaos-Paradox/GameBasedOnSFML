#pragma once

/**
 * @brief 物品数据组件（纯数据）
 *
 * ⚠️ 重构（ECS 纯净原则）：
 * - magnetImmunityTimer 已移除 → 磁吸免疫由 MagnetSystem 内部维护
 *
 * @see PickupSystem - 拾取判定系统
 * @see MagnetSystem - 磁吸系统
 */
struct ItemDataComponent {
    unsigned int itemId{0};
    int amount{1};
    bool isPickupable{true};
};

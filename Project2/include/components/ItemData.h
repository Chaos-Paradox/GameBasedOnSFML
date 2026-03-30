#pragma once

/**
 * @brief 物品数据组件（纯数据，POD 合规）
 * 
 * 掉落物或可交互物品的数据
 * 
 * ⚠️ 修复：简化字段，移除冗余
 * 
 * @see PickupSystem - 拾取判定系统
 */
struct ItemDataComponent {
    unsigned int itemId{0};  // 物品 ID
    int amount{1};           // 数量
    bool isPickupable{true}; // 是否可拾取
};

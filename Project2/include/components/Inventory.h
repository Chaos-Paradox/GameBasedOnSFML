#pragma once
#include <vector>

/**
 * @brief 背包格子（纯数据）
 */
struct InventorySlot {
    unsigned int itemId{0};
    int count{0};
    int maxStack{99};
};

/**
 * @brief 背包组件（纯数据）
 * 
 * 玩家背包。本质是一个物品 ID 到数量的映射
 * 
 * @see docs/01_DATA_SCHEMA.md - 数据字典规范
 * @see InventorySystem - 背包系统
 */
struct InventoryComponent {
    std::vector<InventorySlot> slots;
    int maxSlots{20}; // 背包格子数
};

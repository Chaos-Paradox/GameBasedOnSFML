#pragma once

/**
 * @brief 掉落条目（纯数据）
 */
struct LootEntry {
    unsigned int itemId{0};
    float dropChance{0.0f};  // 掉落概率（0-1）
    int minCount{1};
    int maxCount{1};
    
    // ← 新增：磁吸参数
    float magnetRadius{0.0f};  // 磁吸半径（0=关闭）
    float magnetSpeed{400.0f}; // 磁吸速度
};

/**
 * @brief 掉落表组件（纯数据，POD 合规）
 * 
 * 挂载在怪物上，记录死亡时掉落的物品池
 * 
 * ⚠️ 修复：使用固定数组代替 std::vector，符合 POD 原则
 * 
 * @see LootSpawnSystem - 掉落生成系统
 */
struct LootDropComponent {
    static constexpr int MAX_LOOT_ENTRIES = 8;
    
    LootEntry lootTable[MAX_LOOT_ENTRIES];  // 掉落表
    int lootCount{0};                        // 实际掉落条目数
    bool hasDropped{false};                  // 是否已掉落（防止重复）
};

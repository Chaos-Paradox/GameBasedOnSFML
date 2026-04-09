#pragma once

/**
 * @brief 物品数据组件（纯数据，POD 合规）
 * 
 * 掉落物或可交互物品的数据
 * 
 * @see PickupSystem - 拾取判定系统
 */
struct ItemDataComponent {
    unsigned int itemId{0};
    int amount{1};
    bool isPickupable{true};
    
    // ← 新增：磁吸免疫计时器（爆米花效果）
    float magnetImmunityTimer{0.25f};  // 0.25 秒内不被磁吸（更短的抛射时间）
};

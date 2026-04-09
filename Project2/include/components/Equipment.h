#pragma once

/**
 * @brief 装备组件（纯数据）
 * 
 * 装备槽。记录当前装备的 ItemID
 * 
 * @see docs/01_DATA_SCHEMA.md - 数据字典规范
 * @see EquipmentSystem - 装备系统
 */
struct EquipmentComponent {
    unsigned int head{0};      // 头部装备 ID
    unsigned int body{0};      // 身体装备 ID
    unsigned int weapon{0};    // 武器 ID
    unsigned int accessory{0}; // 饰品 ID
};

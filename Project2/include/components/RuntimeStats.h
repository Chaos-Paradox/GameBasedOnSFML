#pragma once

/**
 * @brief 运行时统计组件（纯数据）
 * 
 * 由 ModifierSystem 每帧/按需计算得出，供战斗系统读取
 * 
 * 计算公式：基础属性 + 装备加成 + 进化增益 = 最终属性
 * 
 * @see docs/01_DATA_SCHEMA.md - 数据字典规范
 * @see ModifierSystem - 属性计算系统
 */
struct RuntimeStatsComponent {
    // 最终战斗面板
    int finalAttack{10};
    int finalDefense{5};
    float finalMoveSpeed{150.0f};
    int finalMaxHP{100};
    
    // 元素属性
    int attackElement{0}; // 0=Physical, 1=Fire, 2=Toxic, 3=Ice
    
    // 元素抗性（0-100，百分比）
    float fireResist{0.0f};
    float toxicResist{0.0f};
    float iceResist{0.0f};
    
    // 当前值（战斗中动态变化）
    int currentHP{100};
};

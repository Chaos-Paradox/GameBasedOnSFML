#pragma once
#include <string>
#include <vector>

/**
 * @brief 属性修饰器组件（纯数据）
 * 
 * 存储临时增益/减益效果，由 ModifierSystem 管理
 * 
 * @see docs/01_DATA_SCHEMA.md - 数据字典规范
 * @see ModifierSystem - 属性计算系统
 */
struct StatModifier {
    enum class Type : unsigned char {
        Flat,           // 固定值（+10 攻击力）
        Percent,        // 百分比（+10% 移速）
        Multiplicative  // 乘算（x1.5 伤害）
    };
    
    Type modifierType{Type::Flat};
    std::string statName;     // "attack", "defense", "moveSpeed"
    float value{0.0f};
    unsigned int sourceId{0};     // 来源（装备 ID、技能 ID 等）
    float duration{0.0f};     // 持续时间（0=永久）
    float remainingTime{0.0f}; // 剩余时间
};

/**
 * @brief 属性修饰器容器组件
 * 
 * 存储所有激活的修饰器
 */
struct StatModifierComponent {
    std::vector<StatModifier> modifiers;
};

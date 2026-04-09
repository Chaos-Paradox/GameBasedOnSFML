#pragma once
#include <string>

/**
 * @brief 角色组件（纯数据）
 * 
 * 存储角色基础属性，不受装备/进化影响
 * 最终战斗面板由 RuntimeStatsComponent 存储
 * 
 * @see docs/01_DATA_SCHEMA.md - 数据字典规范
 */
struct CharacterComponent {
    std::string name;           ///< 角色名称
    int level{1};               ///< 等级
    
    // 基础面板（不受装备/进化影响）
    int maxHP{100};
    int currentHP{100};
    int baseAttack{10};
    int baseDefense{5};
    float baseMoveSpeed{150.0f};
    
    // 运行时状态标志
    bool isInvincible{false};
    float invincibleTimer{0.0f};
    
    // 朝向（2D 向量）
    float facingX{1.0f};
    float facingY{0.0f};
};

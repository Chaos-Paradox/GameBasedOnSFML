#pragma once
#include <string>

/**
 * @brief 角色组件（纯数据）
 *
 * ⚠️ 重构（ECS 纯净原则）：
 * - isInvincible / invincibleTimer 描述"未来行为"（临时 buff 状态）
 *   → 移除，无敌状态由 StateMachineComponent.currentState 或 DashComponent.isInvincible 表达
 * - facingX / facingY 是空间朝向 → 已移至 TransformComponent
 *
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
};

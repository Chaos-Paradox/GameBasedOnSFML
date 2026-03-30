#pragma once

/**
 * @brief 进化组件（纯数据，POD 合规）
 * 
 * 存储角色的进化数据，包括进化点数、技能树、被动突变等
 * 
 * ⚠️ 修复：移除 std::vector 和 std::unordered_map，使用固定数组
 * 
 * @see PickupSystem - 拾取判定系统（增加进化点数）
 */
struct EvolutionComponent {
    // 进化点数
    int evolutionPoints{0};      // 可用进化点数
    int totalEarned{0};          // 累计获得点数（用于统计）
    
    // ← 简化：暂时只存储点数，技能树等后续扩展
    // 固定数组版本（如果需要）：
    // static constexpr int MAX_SKILLS = 16;
    // unsigned int activeSkills[MAX_SKILLS];
    // int activeSkillCount{0};
};

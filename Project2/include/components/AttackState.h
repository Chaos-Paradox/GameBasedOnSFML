#pragma once

/**
 * @brief 攻击状态组件（纯数据）
 * 
 * 🔄 重构（网游 ARPG 风格 - 扇形几何瞬时扫描）：
 * - 不再依赖物理 Hitbox 实体（网络同步灾难）
 * - 改为纯数学距离+向量点乘的一次性伤害结算
 * - hasFiredDamage 作为单次判定锁，确保一次动画只结算一次
 * 
 * @see StateMachineSystem - 添加/移除组件
 * @see AttackSystem - 执行扇形扫描
 */
struct AttackStateComponent {
    float hitTimer{0.0f};       ///< 攻击计时器
    float hitDuration{0.3f};    ///< 攻击持续时间
    int baseDamage{10};         ///< 基础伤害值

    float attackRange{120.0f};  ///< 攻击半径（扇形扫描范围）
    float attackArc{120.0f};    ///< 扇形夹角（角度）
    bool hasFiredDamage{false}; ///< ← 单次判定锁：一次挥砍只结算一次伤害
};

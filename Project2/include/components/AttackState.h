#pragma once

/**
 * @brief 攻击状态组件（纯数据）
 * 
 * ⚠️ Bug 1 修复关键：作为"单次触发锁"
 * - 进入 Attack 状态时添加
 * - 离开 Attack 状态时移除
 * - hitActivated 确保只创建一次 Hitbox
 * 
 * @see StateMachineSystem - 添加/移除组件
 * @see AttackSystem - 检查 hitActivated
 */
struct AttackStateComponent {
    float hitTimer{0.0f};       ///< 攻击计时器
    float hitDuration{0.3f};    ///< 攻击持续时间
    bool hitActivated{false};   ///< ← 关键：是否已激活 Hitbox（单次锁）
};

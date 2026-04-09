#pragma once

/**
 * @brief 伤害标签（单帧事件，用于跨系统通信）
 * 
 * CombatSystem 写入 → StateMachineSystem 读取 → 帧末销毁
 * 
 * @see docs/00_ARCHITECTURE.md - 数据交接协议
 */
struct DamageTag {
    float damage{0.0f};  ///< 伤害值
};

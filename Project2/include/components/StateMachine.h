#pragma once
#include "../states/IState.h"

// CharacterState 枚举定义
enum class CharacterState : uint8_t {
    Idle,       // 待机
    Move,       // 移动
    Attack,     // 攻击
    Dash,       // 冲刺
    Hurt,       // 受伤
    Dead,       // 死亡
    KnockedAirborne,  // ← 新增：击飞浮空（硬控状态）
};

/**
 * @brief 状态机组件（纯数据）
 * 
 * ⚠️ 注意：本组件只存储状态数据，不包含任何逻辑
 * 状态切换逻辑在 StateMachineSystem 中执行
 * 
 * @see StateMachineSystem - 状态机系统
 * @see docs/01_DATA_SCHEMA.md - 数据字典规范
 */
struct StateMachineComponent {
    CharacterState currentState{CharacterState::Idle};   ///< 当前状态
    CharacterState previousState{CharacterState::Idle};  ///< 上一个状态（用于恢复）
    float stateTimer{0.0f};  ///< 当前状态持续时间（秒）
};

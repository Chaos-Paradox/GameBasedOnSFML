#pragma once
#include "../math/Vec2.h"

/**
 * @brief 冲刺组件（纯数据）
 * 
 * ⚠️ 关键设计：冲刺状态由 CharacterState::Dash 表示，不是 isDashing！
 * isInvincible 只用于渲染无敌帧视觉效果。
 */
struct DashComponent {
    // 冲刺参数
    float dashSpeed{2000.0f};       // 冲刺初速度（像素/秒）
    float dashDuration{0.1f};       // ← 冲刺持续时间（秒）减半到 0.1 秒
    float iframeDuration{0.1f};     // ← 无敌帧时间（秒）减半到 0.1 秒
    float cooldown{1.0f};           // 冷却时间（秒）
    
    // 运行时状态
    float dashTimer{0.0f};          // 当前冲刺进度（秒）
    float cooldownTimer{0.0f};      // 当前冷却进度（秒）
    float iframeTimer{0.0f};        // 当前无敌时间（秒）
    Vec2 dashDir{0.0f, 0.0f};       // 锁定冲刺方向
    
    // 状态标志（只保留 isInvincible 用于渲染）
    bool isInvincible{false};       // 是否处于无敌帧（用于渲染青色半透明）
};

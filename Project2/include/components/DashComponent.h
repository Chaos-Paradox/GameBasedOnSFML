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
    float dashSpeed{2000.0f};       // 冲刺最高速度（像素/秒）
    float dashDuration{0.3f};       // 冲刺持续时间（秒）
    float iframeDuration{0.18f};    // 无敌帧时间（秒）——期间保持最高速度
    float recoveryDuration{0.0f};   // 后摇时间（秒）——设为 0 表示无后摇

    // 充能参数
    int maxCharges{2};              // 最大充能次数
    int currentCharges{2};          // 当前充能次数
    float rechargeCooldown{1.0f};   // 单次充能恢复时间（秒）

    // 运行时状态
    float dashTimer{0.0f};          // 当前冲刺进度（秒）
    float rechargeTimer{0.0f};      // 充能计时器（归零时获得 1 次充能）
    float iframeTimer{0.0f};        // 当前无敌时间（秒）
    float recoveryTimer{0.0f};      // 后摇倒计时（秒），进入后摇时设置
    Vec2 dashDir{0.0f, 0.0f};       // 锁定冲刺方向

    // 状态标志
    bool isInvincible{false};       // 是否处于无敌帧（用于渲染青色半透明）
};

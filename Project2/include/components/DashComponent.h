#pragma once
#include "../math/Vec2.h"

/**
 * @brief 冲刺状态组件（纯数据）
 *
 * ⚠️ 重构（ECS 纯净原则）：
 * - 组件只描述"当前是什么"，不描述"行为"或"未来"
 * - cooldown 逻辑由 DashSystem 内部计时，不存入组件
 * - 充能次数是"当前有多少"，是合法的状态描述
 *
 * @see DashSystem - 充能恢复、冲刺状态推进
 */
struct DashComponent {
    // 冲刺参数
    float dashSpeed{2000.0f};       // 冲刺最高速度（像素/秒）
    float dashDuration{0.3f};       // 冲刺持续时间（秒）
    float iframeDuration{0.18f};    // 无敌帧时间（秒）——期间保持最高速度
    float recoveryDuration{0.0f};   // 后摇时间（秒）——设为 0 表示无后摇

    // 充能参数（描述"当前有多少"，合法）
    int maxCharges{2};              // 最大充能次数
    int currentCharges{2};          // 当前充能次数
    float rechargeCooldown{1.0f};   // 单次充能恢复间隔（秒），配置参数

    // 运行时状态（描述"当前进度"，合法）
    float dashTimer{0.0f};          // 当前冲刺进度（秒）
    float iframeTimer{0.0f};        // 当前无敌时间（秒）
    float recoveryTimer{0.0f};      // 后摇倒计时（秒），进入后摇时设置
    Vec2 dashDir{0.0f, 0.0f};       // 锁定冲刺方向

    // 状态标志（描述"当前是否"，合法）
    bool isInvincible{false};       // 是否处于无敌帧（用于渲染青色半透明）

    // 充能计时（描述"当前充能进度"，合法）
    float rechargeTimer{0.0f};      // 充能计时器（归零时获得 1 次充能）
};

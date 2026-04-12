#pragma once

/**
 * @brief GameJuice 全局打击感状态器
 *
 * 控制时间流速（顿帧 Hit-Stop）和屏幕震动（Camera Shake）
 * 由各 System 触发，由 VisualSandbox 主循环消费
 */
struct GameJuice {
    float timeScale{1.0f};         // 游戏世界时间流速
    float hitStopTimer{0.0f};      // 顿帧剩余时间（真实时间）
    float shakeTimer{0.0f};        // 震动剩余时间（真实时间）
    float shakeIntensity{0.0f};    // 当前震动强度（像素偏移量）
};

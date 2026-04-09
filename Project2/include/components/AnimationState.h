#pragma once
#include <string>

/**
 * @brief 动画状态组件（纯数据）
 * 
 * 当前播放的动画状态，供渲染层读取
 * 
 * @see docs/01_DATA_SCHEMA.md - 数据字典规范
 * @see AnimationSystem - 动画系统
 */
struct AnimationStateComponent {
    std::string currentAnimation;
    bool isPlaying{false};
    float playbackTime{0.0f};
};

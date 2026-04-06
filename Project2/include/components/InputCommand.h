#pragma once
#include "../math/Vec2.h"

/**
 * @brief 输入命令组件（纯数据）
 * 
 * ⚠️ 关键设计：使用计时器代替布尔值，实现限时输入缓存
 * 
 * @see StateMachineSystem - 消费输入缓存
 */
struct InputCommand {
    Vec2 moveDir{0.0f, 0.0f};       // 移动方向（连续向量）
    float attackBufferTimer{0.0f};  // ← 攻击指令缓存计时器（秒）
                                    //   > 0 表示有有效的攻击指令
                                    //   每帧减少，0.2 秒后过期
};

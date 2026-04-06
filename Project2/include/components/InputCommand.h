#pragma once
#include "../math/Vec2.h"

/**
 * @brief 动作意图枚举（单轨指令槽）
 * 
 * ⚠️ 工业级设计：单一意图槽 + Last-In-Wins 覆盖原则
 *   - 废弃独立 Timer，改用统一 intentTimer
 *   - 新指令无条件覆盖旧指令，并重置保质期
 *   - 僵直期间计时器暂停（时间静止魔法）
 * 
 * @see StateMachineSystem - 消费输入缓存
 */
enum class ActionIntent {
    None,
    Attack,
    Dash
};

/**
 * @brief 输入命令组件（纯数据）
 * 
 * @see StateMachineSystem - 消费输入缓存
 */
struct InputCommand {
    Vec2 moveDir{0.0f, 0.0f};           // 移动方向（连续向量）
    ActionIntent pendingIntent{ActionIntent::None};  // ← 单一意图槽
    float intentTimer{0.0f};            // ← 意图保质期（秒）
                                        //   僵直期间暂停倒计时
                                        //   只在可行动状态下减少
};

#pragma once
#include "../math/Vec2.h"

/**
 * @brief 输入命令组件（纯数据）
 * 
 * ⚠️ 架构升级：从"离散指令"转向"连续向量"
 * 
 * 旧设计问题：
 * - Command enum 具有排他性（只能是上下左右中的一种）
 * - 无法表示斜向移动
 * - if-return 链导致按键互斥
 * 
 * 新设计优势：
 * - Vec2 可以表示任意方向
 * - 支持斜向移动（W+D = (1, -1)）
 * - 向量累加，自然支持多键同时按下
 * 
 * @see LocomotionSystem - 读取 moveDir 并归一化
 */
struct InputCommand {
    Vec2 moveDir{0.0f, 0.0f};  // ← 取代原来的 Command cmd
    bool attackPressed{false};
};

#pragma once
#include "../math/Vec2.h"
#include <string>

/**
 * @brief 伤害飘字组件（纯视觉）
 * 
 * 用于显示伤害数字的飘字效果
 * 
 * 生命周期：约 1 秒
 * - DamageTextSpawnerSystem 创建
 * - DamageTextRenderSystem 更新并渲染
 * - timer <= 0 时自动销毁
 * 
 * @see DamageTextSpawnerSystem - 生成飘字
 * @see DamageTextRenderSystem - 渲染飘字
 */
struct DamageTextComponent {
    std::string text{""};           // 显示的文字（如 "15" 或 "20!"）
    float timer{1.0f};              // 存活倒计时（秒）
    Vec2 position{0.0f, 0.0f};      // 当前坐标（会向上漂浮）
    Vec2 velocity{0.0f, -50.0f};    // 漂浮速度（向上）
    bool isCritical{false};         // 是否暴击（决定颜色）
    float alpha{1.0f};              // 透明度（1.0=完全不透明，0=完全透明）
    
    // 显示配置
    float fontSize{24.0f};          // 字体大小
    float fadeOutStart{0.5f};       // 开始淡出的时间（剩余秒数）
};

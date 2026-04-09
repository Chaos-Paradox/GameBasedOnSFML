#pragma once
#include <string>
#include "../math/Vec2.h"

/**
 * @brief 渲染事件组件（纯数据）
 * 
 * 供外部渲染引擎读取。Project2 逻辑层不处理此组件。
 * 
 * @see docs/01_DATA_SCHEMA.md - 数据字典规范
 * @see docs/00_ARCHITECTURE.md - 渲染解耦规范
 */
struct RenderEventComponent {
    enum class Type : unsigned char {
        PlayAnimation,
        PlaySound,
        SpawnParticle,
        StopSound,
    };
    
    Type type{Type::PlayAnimation};
    std::string assetKey;       // 资源键（动画名、音效名等）
    Vec2 position{0.0f, 0.0f};
    float volume{1.0f};         // 音量（0-1，用于音效）
};

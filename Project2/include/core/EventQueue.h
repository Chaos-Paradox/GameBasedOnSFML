#pragma once

#include "core/Entity.h"
#include "math/Vec2.h"
#include <vector>

// ============================================================
// EventQueue — 事件队列
//
// Component = 世界是什么（当前状态快照）
// System    = 世界如何变化（t → t+1 的推进）
// Event     = 世界刚刚发生了什么（瞬时信号）
//
// 所有事件必须走 EventQueue，不得以 Component 形式进入 ECS。
// 事件生命周期：本帧写入 → 下一帧 System 消费 → 清空
// ============================================================

// ---------- 伤害事件 ----------
struct DamageEvent {
    Entity target{INVALID_ENTITY};
    int actualDamage{0};
    Vec2 hitPosition{0.0f, 0.0f};
    bool isCritical{false};
    Vec2 hitDirection{0.0f, 0.0f};
    float knockbackXY{0.0f};
    float knockbackZ{0.0f};
    Entity attacker{INVALID_ENTITY};
};

// ---------- 渲染事件 ----------
struct RenderEvent {
    enum class Type : unsigned char {
        PlayAnimation,
        PlaySound,
        SpawnParticle,
        StopSound,
    };

    Type type{Type::PlayAnimation};
    const char* assetKey{nullptr};  // 资源键（动画名、音效名等）
    Vec2 position{0.0f, 0.0f};
    float volume{1.0f};
};

// ---------- 伤害飘字事件 ----------
struct DamageTextEvent {
    int damage{0};
    bool isCritical{false};
    Vec2 position{0.0f, 0.0f};
};

// ---------- 爆炸事件 ----------
struct ExplosionEvent {
    Vec2 position{0.0f, 0.0f};
    float radius{0.0f};
    int damage{0};
    Entity source{INVALID_ENTITY};
};

// ============================================================
// EventQueue — 全局事件队列，挂载在 GameWorld 上
// ============================================================
struct EventQueue {
    // 本帧产生的事件
    std::vector<DamageEvent>      damageEvents;
    std::vector<DamageTextEvent>  damageTextEvents;
    std::vector<RenderEvent>      renderEvents;
    std::vector<ExplosionEvent>   explosionEvents;

    // 帧末清空，确保事件不跨帧残留
    void endFrame() {
        damageEvents.clear();
        damageTextEvents.clear();
        renderEvents.clear();
        explosionEvents.clear();
    }
};

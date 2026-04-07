#pragma once
#include "../math/Vec2.h"
#include "../core/Entity.h"

/**
 * @brief 伤害事件组件（事件实体载荷）
 * 
 * ⚠️ 关键设计：使用事件实体而非 Tag，支持：
 * - 伤害随机浮动（0.8f ~ 1.2f）
 * - 多重打击同步（多个事件实体并行）
 * - 延迟结算（可在下一帧或指定时机处理）
 * 
 * 生命周期：1 帧
 * - CollisionSystem 创建事件实体
 * - DamageSystem 读取并结算
 * - CleanupSystem 销毁事件实体
 * 
 * @see CollisionSystem - 创建伤害事件
 * @see DamageSystem - 结算伤害
 */
struct DamageEventComponent {
    Entity target{INVALID_ENTITY};    // 受击者实体 ID
    int actualDamage{0};              // 结算后的最终伤害（已计算浮动）
    Vec2 hitPosition{0.0f, 0.0f};     // 打击发生的世界坐标
    bool isCritical{false};           // 是否暴击（浮动倍率 > 1.1f）
    
    // ← 【新增】击飞参数（支持战斗物理）
    Vec2 hitDirection{0.0f, 0.0f};    // 力的方向（归一化）
    float knockbackXY{0.0f};          // 水平击飞力度
    float knockbackZ{0.0f};           // 垂直挑飞力度
    
    // 来源信息（可选）
    Entity attacker{INVALID_ENTITY};  // 攻击者实体 ID
    float timestamp{0.0f};            // 事件创建时间（用于调试）
};

#pragma once

/**
 * @brief 碰撞层级定义
 * 
 * 使用位掩码，支持 32 个层级
 * 用于过滤哪些物体可以相互碰撞
 */
namespace CollisionLayer {
    constexpr int DEFAULT    = 1 << 0;  // 默认层级
    constexpr int PLAYER     = 1 << 1;  // 玩家
    constexpr int ENEMY      = 1 << 2;  // 敌人
    constexpr int PROJECTILE = 1 << 3;  // 投射物
    constexpr int TRIGGER    = 1 << 4;  // 触发器（无碰撞）
    constexpr int ENVIRONMENT = 1 << 5; // 环境物体
}

/**
 * @brief Hitbox 类型
 * 
 * 区分不同类型的攻击
 */
enum class HitboxType {
    None,       // 无效
    Melee,      // 近战攻击
    Ranged,     // 远程攻击
    Skill,      // 技能
    Environment // 环境伤害
};

/**
 * @brief 阵营定义
 * 
 * 用于区分友军和敌军
 */
enum class Faction {
    None,
    Player,
    Enemy,
    Neutral
};

/**
 * @brief 碰撞事件
 * 
 * 当 Hitbox 与 Hurtbox 碰撞时生成
 */
struct CollisionEvent {
    Entity attacker = INVALID_ENTITY;  // 攻击者实体
    Entity victim = INVALID_ENTITY;    // 受害者实体
    float damage = 0.0f;               // 伤害值
    HitboxType type = HitboxType::None; // 攻击类型
    int hitboxLayer = 0;               // Hitbox 层级
    int hurtboxLayer = 0;              // Hurtbox 层级
};

/**
 * @brief 碰撞配置
 * 
 * 定义哪些层级可以相互碰撞
 */
struct CollisionConfig {
    int layer = CollisionLayer::DEFAULT;  // 当前层级
    int mask = 0xFFFFFFFF;                // 碰撞掩码（哪些层级可以碰撞）
    
    /**
     * @brief 检查是否可以与另一个层级碰撞
     */
    bool canCollideWith(int otherLayer) const {
        return (mask & otherLayer) != 0;
    }
};

/**
 * @brief 快速层级配置工具
 */
namespace CollisionMatrix {
    // 玩家 vs 敌人
    constexpr int PLAYER_DAMAGE_ENEMY = CollisionLayer::PLAYER | CollisionLayer::ENEMY;
    constexpr int ENEMY_DAMAGE_PLAYER = CollisionLayer::ENEMY | CollisionLayer::PLAYER;
    
    // 投射物 vs 所有
    constexpr int PROJECTILE_DAMAGE_ALL = CollisionLayer::PROJECTILE | 
                                          CollisionLayer::PLAYER | 
                                          CollisionLayer::ENEMY;
}

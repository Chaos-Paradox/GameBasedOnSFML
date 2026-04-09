#pragma once
#include "DamageTag.h"
#include "../core/Entity.h"

/**
 * @brief 进化指令 Tag
 * 
 * UI 层 → ProgressionSystem
 */
struct EvolveCommandTag {
    uint32_t skillId{0};  ///< 技能/突变 ID
    int level{1};         ///< 升级等级（1=解锁/升级 1 级）
};

/**
 * @brief 交互指令 Tag
 * 
 * 玩家输入 → InteractionSystem
 */
struct InteractCommandTag {
    EntityId targetEntity{INVALID_ENTITY_ID};  ///< 想要交互的掉落物或 NPC
};

/**
 * @brief 合成指令 Tag
 * 
 * 玩家输入 → CraftingSystem
 */
struct CraftCommandTag {
    uint32_t recipeId{0};  ///< 配方 ID
    int craftCount{1};     ///< 合成数量
};

/**
 * @brief 生成指令 Tag
 * 
 * DirectorSystem → World
 */
struct SpawnCommandTag {
    uint32_t enemyTypeId{0};           ///< 敌人生成类型
    sf::Vector2f spawnPosition{0.0f, 0.0f};  ///< 生成位置
    int count{1};                      ///< 生成数量
};

#pragma once
#include "../core/Component.h"
#include "../components/DamageTag.h"
#include "../components/DeathTag.h"
#include "../components/Character.h"
#include <iostream>

/**
 * @brief 伤害结算系统
 * 
 * 职责：
 * - 扫描所有 DamageTag
 * - 扣减 CharacterComponent.currentHP
 * - HP ≤ 0 时挂载 DeathTag（死亡通知单）
 * - 立即销毁 DamageTag（防止重复伤害）
 */
class DamageSystem {
public:
    void update(
        ComponentStore<CharacterComponent>& characters,
        ComponentStore<DamageTag>& damageTags,
        ComponentStore<DeathTag>& deathTags)
    {
        auto entities = damageTags.entityList();
        for (Entity entity : entities) {
            const auto& tag = damageTags.get(entity);
            
            if (characters.has(entity)) {
                auto& character = characters.get(entity);
                int oldHP = character.currentHP;
                character.currentHP -= static_cast<int>(tag.damage);
                
                // 死亡检测
                if (character.currentHP <= 0) {
                    character.currentHP = 0;
                    
                    // ← 【修复】关键：贴上 DeathTag（死亡通知单）
                    if (!deathTags.has(entity)) {
                        deathTags.add(entity, {0.0f});
                        std::cout << "[DamageSystem] DeathTag added to Entity " << entity << "\n";
                    }
                } else {
                    std::cout << "[DamageSystem] Entity " << entity 
                              << " took " << (oldHP - character.currentHP) 
                              << " damage (HP: " << oldHP << " -> " << character.currentHP << ")\n";
                }
            }
            
            // 销毁 DamageTag
            damageTags.remove(entity);
        }
    }
};

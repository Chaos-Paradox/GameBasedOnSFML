#pragma once
#include "../core/Component.h"
#include "../core/ECS.h"
#include "../components/DeathTag.h"
#include "../components/Lifetime.h"
#include "../components/StateMachine.h"
#include "../components/Transform.h"
#include "../components/Character.h"
#include "../components/InputCommand.h"
#include "../components/Hurtbox.h"
#include "../components/Hitbox.h"
#include "../components/AttackState.h"
#include "../components/DamageTag.h"
#include "../components/LootDrop.h"
#include "../components/ItemData.h"
#include "../components/PickupBox.h"
#include "../components/MagnetComponent.h"
#include "../components/Evolution.h"
#include "../components/DashComponent.h"
#include "../components/BombComponent.h"
#include "../components/AttachedComponent.h"
#include "../components/ColliderComponent.h"
#include "../components/ZTransformComponent.h"
#include "../components/DamageTextComponent.h"
#include <vector>
#include <algorithm>

/**
 * @brief 清理系统（延迟销毁 + 孟婆汤）
 * 
 * ⚠️ 架构设计：
 * - 在帧末统一清理所有带 DeathTag 的实体
 * - 清理到期的 Lifetime 实体
 * - 【核心】销毁前清空所有组件数据（防止实体复用污染）
 * - 防止 std::out_of_range 崩溃
 * 
 * 执行时机：主循环最末尾（渲染之后）
 * 
 * @see DeathTag - 死亡标记
 * @see LifetimeComponent - 寿命组件
 */
class CleanupSystem {
public:
    void update(
        ECS& ecs,
        ComponentStore<DeathTag>& deathTags,
        ComponentStore<LifetimeComponent>& lifetimes,
        ComponentStore<StateMachineComponent>& states,
        ComponentStore<TransformComponent>& transforms,
        ComponentStore<CharacterComponent>& characters,
        ComponentStore<InputCommand>& inputs,
        ComponentStore<HurtboxComponent>& hurtboxes,
        ComponentStore<HitboxComponent>& hitboxes,
        ComponentStore<AttackStateComponent>& attackStates,
        ComponentStore<DamageTag>& damageTags,
        ComponentStore<LootDropComponent>& lootDrops,
        ComponentStore<ItemDataComponent>& itemDatas,
        ComponentStore<PickupBoxComponent>& pickupBoxes,
        ComponentStore<MagnetComponent>& magnets,
        ComponentStore<EvolutionComponent>& evolutions,
        ComponentStore<DashComponent>& dashes,
        ComponentStore<BombComponent>& bombs,
        ComponentStore<AttachedComponent>& attachedComponents,
        ComponentStore<ColliderComponent>& colliders,
        ComponentStore<ZTransformComponent>& zTransforms,
        ComponentStore<DamageTextComponent>& damageTexts,  // ← 新增：伤害飘字清理
        float dt)
    {
        // ========== 1. 收集所有需要销毁的实体 ==========
        std::vector<Entity> entitiesToDestroy;
        
        // 收集 DeathTag 实体
        auto deathEntities = deathTags.entityList();
        for (Entity entity : deathEntities) {
            // 移除 DeathTag（防止重复处理）
            deathTags.remove(entity);
            entitiesToDestroy.push_back(entity);
        }
        
        // 收集到期 Lifetime 实体
        auto lifetimeEntities = lifetimes.entityList();
        for (Entity entity : lifetimeEntities) {
            if (!lifetimes.has(entity)) continue;
            
            auto& lifetime = lifetimes.get(entity);
            lifetime.timeLeft -= dt;
            
            if (lifetime.timeLeft <= 0.0f && lifetime.autoDestroy) {
                // 避免重复添加
                if (std::find(entitiesToDestroy.begin(), entitiesToDestroy.end(), entity) == entitiesToDestroy.end()) {
                    entitiesToDestroy.push_back(entity);
                }
            }
        }
        
        // ========== 2. 统一执行抹除与销毁 (绝不漏掉任何一个 ComponentStore) ==========
        for (Entity entity : entitiesToDestroy) {
            // 【核心】灌入孟婆汤：清空所有组件数据
            states.remove(entity);
            transforms.remove(entity);
            characters.remove(entity);
            inputs.remove(entity);
            hurtboxes.remove(entity);
            
            // ⚠️ 极其关键：必须清除 hitbox，并确保其内部的 unordered_set 也被清空！
            hitboxes.remove(entity);
            
            attackStates.remove(entity);
            lifetimes.remove(entity);
            damageTags.remove(entity);
            deathTags.remove(entity);  // 清理死亡标记自己
            lootDrops.remove(entity);
            itemDatas.remove(entity);
            pickupBoxes.remove(entity);
            magnets.remove(entity);
            evolutions.remove(entity);
            dashes.remove(entity);
            bombs.remove(entity);
            attachedComponents.remove(entity);
            colliders.remove(entity);
            zTransforms.remove(entity);
            damageTexts.remove(entity);  // ← 新增：清理伤害飘字
            
            // ← 【调试】打印销毁日志
            std::cout << "[CLEANUP] 🧹 Wiping and Destroying ID: " << (uint32_t)entity << std::endl;
            
            // 抹除所有记忆后，安全销毁 ID
            ecs.destroy(entity);
        }
    }
};

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
#include <vector>

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
        float dt)
    {
        // ========== 1. 清理 DeathTag 实体 ==========
        auto deathEntities = deathTags.entityList();
        
        // 复制列表，避免迭代中修改
        std::vector<Entity> toDestroy(deathEntities.begin(), deathEntities.end());
        
        for (Entity entity : toDestroy) {
            // 【核心】灌入孟婆汤：清空所有组件数据
            states.remove(entity);
            transforms.remove(entity);
            characters.remove(entity);
            inputs.remove(entity);
            hurtboxes.remove(entity);
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
            
            // 抹除所有组件后，再销毁实体 ID
            ecs.destroy(entity);
        }
        
        // ========== 2. 清理到期 Lifetime 实体 ==========
        auto lifetimeEntities = lifetimes.entityList();
        
        for (Entity entity : lifetimeEntities) {
            if (!lifetimes.has(entity)) continue;
            
            auto& lifetime = lifetimes.get(entity);
            lifetime.timeLeft -= dt;
            
            if (lifetime.timeLeft <= 0.0f) {
                if (lifetime.autoDestroy) {
                    // 【核心】灌入孟婆汤：清空所有组件数据
                    states.remove(entity);
                    transforms.remove(entity);
                    characters.remove(entity);
                    inputs.remove(entity);
                    hurtboxes.remove(entity);
                    hitboxes.remove(entity);
                    attackStates.remove(entity);
                    lifetimes.remove(entity);
                    damageTags.remove(entity);
                    deathTags.remove(entity);
                    lootDrops.remove(entity);
                    itemDatas.remove(entity);
                    pickupBoxes.remove(entity);
                    magnets.remove(entity);
                    evolutions.remove(entity);
                    dashes.remove(entity);
                    bombs.remove(entity);
                    attachedComponents.remove(entity);
                    colliders.remove(entity);
                    
                    // 抹除所有组件后，再销毁实体 ID
                    ecs.destroy(entity);
                }
            }
        }
    }
};

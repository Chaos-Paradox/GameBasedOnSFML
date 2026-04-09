#pragma once
#include "../math/Vec2.h"
#include "../core/Entity.h"
#include "../core/ECS.h"
#include "../core/Component.h"
#include "../components/Transform.h"
#include "../components/Character.h"
#include "../components/StateMachine.h"
#include "../components/Inventory.h"

/**
 * @brief 玩家实体工厂
 * 
 * 提供创建标准玩家实体的静态方法
 * 
 * @see docs/01_DATA_SCHEMA.md - 数据字典规范
 */
class PlayerFactory {
public:
    /**
     * @brief 创建玩家实体
     */
    static EntityId createPlayer(
        ECS& ecs,
        ComponentStore<TransformComponent>& transforms,
        ComponentStore<CharacterComponent>& characters,
        ComponentStore<StateMachineComponent>& states,
        ComponentStore<InventoryComponent>& inventories,
        Vec2 startPos)
    {
        EntityId entity = ecs.create();
        
        transforms.add(entity, {
            .position = startPos,
            .scale = {1.0f, 1.0f},
            .rotation = 0.0f,
            .velocity = {0.0f, 0.0f}
        });
        
        characters.add(entity, {
            .name = "Player",
            .level = 1,
            .maxHP = 100,
            .currentHP = 100,
            .baseAttack = 10,
            .baseDefense = 5,
            .baseMoveSpeed = 150.0f,
            .isInvincible = false,
            .invincibleTimer = 0.0f,
            .facingX = 1.0f,
            .facingY = 0.0f
        });
        
        states.add(entity, {
            .currentState = CharacterState::Idle,
            .previousState = CharacterState::Idle,
            .stateTimer = 0.0f
        });
        
        InventoryComponent inventory;
        inventory.slots.reserve(inventory.maxSlots);
        inventories.add(entity, std::move(inventory));
        
        return entity;
    }
    
    /**
     * @brief 创建敌人实体
     */
    static EntityId createEnemy(
        ECS& ecs,
        ComponentStore<TransformComponent>& transforms,
        ComponentStore<CharacterComponent>& characters,
        ComponentStore<StateMachineComponent>& states,
        Vec2 startPos,
        int hp = 50,
        int attack = 8)
    {
        EntityId entity = ecs.create();
        
        transforms.add(entity, {
            .position = startPos,
            .scale = {1.0f, 1.0f},
            .rotation = 0.0f,
            .velocity = {0.0f, 0.0f}
        });
        
        characters.add(entity, {
            .name = "Enemy",
            .level = 1,
            .maxHP = hp,
            .currentHP = hp,
            .baseAttack = attack,
            .baseDefense = 3,
            .baseMoveSpeed = 100.0f,
            .isInvincible = false,
            .invincibleTimer = 0.0f,
            .facingX = -1.0f,
            .facingY = 0.0f
        });
        
        states.add(entity, {
            .currentState = CharacterState::Idle,
            .previousState = CharacterState::Idle,
            .stateTimer = 0.0f
        });
        
        return entity;
    }
    
    /**
     * @brief 创建掉落物实体
     */
    static EntityId createLoot(
        ECS& ecs,
        ComponentStore<TransformComponent>& transforms,
        ComponentStore<ItemDataComponent>& itemData,
        Vec2 startPos,
        unsigned int itemId,
        int count = 1)
    {
        EntityId entity = ecs.create();
        
        transforms.add(entity, {
            .position = startPos,
            .scale = {0.5f, 0.5f},
            .rotation = 0.0f,
            .velocity = {0.0f, 0.0f}
        });
        
        itemData.add(entity, {
            .itemId = itemId,
            .count = count,
            .isPickupable = true
        });
        
        return entity;
    }
};

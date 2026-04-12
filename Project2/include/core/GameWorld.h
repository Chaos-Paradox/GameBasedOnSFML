#pragma once
#include "core/ECS.h"
#include "core/GameJuice.h"
#include "core/InputManager.h"

// 所有组件
#include "components/StateMachine.h"
#include "components/Transform.h"
#include "components/Character.h"
#include "components/InputCommand.h"
#include "components/Hurtbox.h"
#include "components/Hitbox.h"
#include "components/AttackState.h"
#include "components/Lifetime.h"
#include "components/DamageTag.h"
#include "components/DamageEventComponent.h"
#include "components/DeathTag.h"
#include "components/LootDrop.h"
#include "components/ItemData.h"
#include "components/PickupBox.h"
#include "components/MagnetComponent.h"
#include "components/Evolution.h"
#include "components/DashComponent.h"
#include "components/ZTransformComponent.h"
#include "components/ColliderComponent.h"
#include "components/AttachedComponent.h"
#include "components/BombComponent.h"
#include "components/DamageTextComponent.h"

/**
 * @brief 游戏世界上下文
 *
 * 集中管理所有 ECS 组件容器与全局状态，消除 System 间大量参数传递。
 *
 * 使用方式：
 *   GameWorld world;
 *   movementSystem.update(world, dt);
 *   // 内部通过 world.transforms.get(e)、world.ecs.create() 等访问
 */
struct GameWorld {
    ECS ecs;

    // ========== 所有组件容器 ==========
    ComponentStore<StateMachineComponent>   states;
    ComponentStore<TransformComponent>      transforms;
    ComponentStore<CharacterComponent>      characters;
    ComponentStore<InputCommand>            inputs;
    ComponentStore<HurtboxComponent>        hurtboxes;
    ComponentStore<HitboxComponent>         hitboxes;
    ComponentStore<AttackStateComponent>    attackStates;
    ComponentStore<LifetimeComponent>       lifetimes;
    ComponentStore<DamageTag>               damageTags;
    ComponentStore<DamageEventComponent>    damageEvents;
    ComponentStore<DeathTag>                deathTags;
    ComponentStore<LootDropComponent>       lootDrops;
    ComponentStore<ItemDataComponent>       itemDatas;
    ComponentStore<PickupBoxComponent>      pickupBoxes;
    ComponentStore<MagnetComponent>         magnets;
    ComponentStore<EvolutionComponent>      evolutions;
    ComponentStore<DashComponent>           dashes;
    ComponentStore<ZTransformComponent>     zTransforms;
    ComponentStore<ColliderComponent>       colliders;
    ComponentStore<AttachedComponent>       attachedComponents;
    ComponentStore<BombComponent>           bombs;
    ComponentStore<DamageTextComponent>     damageTexts;

    // ========== 全局状态 ==========
    GameJuice juice;

    // ========== 输入管理器（双玩家独立配置） ==========
    InputManager inputManager;

    // ========== 玩家实体引用 ==========
    Entity player1{INVALID_ENTITY};
    Entity player2{INVALID_ENTITY};
};

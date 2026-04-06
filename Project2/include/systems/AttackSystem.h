#pragma once
#include "../core/Component.h"
#include "../core/Entity.h"
#include "../core/ECS.h"
#include "../components/StateMachine.h"
#include "../components/Transform.h"
#include "../components/Character.h"
#include "../components/Hitbox.h"
#include "../components/Lifetime.h"
#include "../components/AttackState.h"
#include "../components/AttachedComponent.h"
#include "../components/ZTransformComponent.h"

/**
 * @brief 攻击判定系统
 * 
 * ⚠️ 关键设计：
 * - 攻击开始第一帧瞬间定身（velocity = 0）
 * - 不施加摩擦力衰减（干脆利落）
 * - hitTimer <= 0 时立刻释放回 Idle（切除后摇）
 */
class AttackSystem {
public:
    void update(
        ComponentStore<StateMachineComponent>& states,
        ComponentStore<AttackStateComponent>& attackStates,
        ComponentStore<TransformComponent>& transforms,
        const ComponentStore<CharacterComponent>& characters,
        ECS& ecs,
        ComponentStore<TransformComponent>& hitboxTransforms,
        ComponentStore<HitboxComponent>& hitboxes,
        ComponentStore<LifetimeComponent>& lifetimes,
        ComponentStore<AttachedComponent>& attachedComponents,
        ComponentStore<ZTransformComponent>& zTransforms,
        float dt)
    {
        (void)characters;
        
        auto entities = attackStates.entityList();
        for (Entity entity : entities) {
            auto& state = states.get(entity);
            auto& attackState = attackStates.get(entity);
            
            // 状态拦截
            if (state.currentState == CharacterState::Dash ||
                state.currentState == CharacterState::Hurt ||
                state.currentState == CharacterState::Dead) {
                continue;
            }
            
            // ← 【核心】攻击状态处理
            if (state.currentState == CharacterState::Attack) {
                // 减少攻击计时器
                attackState.hitTimer -= dt;
                
                // ← 【切除后摇】计时器到 0 立刻释放回 Idle
                if (attackState.hitTimer <= 0.0f) {
                    state.currentState = CharacterState::Idle;
                    state.previousState = CharacterState::Idle;
                    state.stateTimer = 0.0f;
                    
                    std::cout << "[AttackSystem] Attack finished! Released to Idle.\n";
                }
                
                // 已创建过 Hitbox 则跳过
                if (attackState.hitActivated) {
                    continue;
                }
                
                // ← 【瞬间定身】攻击开始第一帧强制 velocity = 0
                if (transforms.has(entity)) {
                    auto& transform = transforms.get(entity);
                    transform.velocity = {0.0f, 0.0f};  // 干脆利落的定身
                }
            } else {
                continue;
            }
            
            // 安全检查
            if (!transforms.has(entity)) {
                continue;
            }
            
            const auto& transform = transforms.get(entity);
            
            // 创建 Hitbox 临时实体
            Entity hitboxEntity = ecs.create();
            
            float offsetX = transform.facingX * 50.0f;
            float offsetY = transform.facingY * 50.0f;
            
            hitboxTransforms.add(hitboxEntity, {
                .position = {transform.position.x + offsetX, transform.position.y + offsetY},
                .scale = {1.0f, 1.0f},
                .rotation = 0.0f,
                .velocity = {0.0f, 0.0f}
            });
            
            hitboxes.add(hitboxEntity, {
                .radius = 35.0f,  // 圆形 Hitbox 半径 35px（近战攻击）
                .offset = {offsetX, offsetY},  // 相对于攻击者的偏移
                .damageMultiplier = 10,
                .element = ElementType::Physical,
                .knockbackForce = 100.0f,
                .sourceEntity = entity,
                .hitHistory = {},
                .hitCount = 0,
                .active = true
            });
            
            // ← 【核心改动】挂载依附组件（同步 XY 位置）
            attachedComponents.add(hitboxEntity, {
                .owner = entity,
                .offset = {offsetX, offsetY}
            });
            
            // ← 【核心改动】同步 Z 轴高度（直接拷贝攻击者的 ZTransform）
            if (zTransforms.has(entity)) {
                const auto& ownerZ = zTransforms.get(entity);
                zTransforms.add(hitboxEntity, {
                    .z = ownerZ.z,
                    .vz = 0.0f,  // Hitbox 不受重力影响
                    .gravity = 0.0f,
                    .height = ownerZ.height  // 与攻击者相同高度
                });
            }
            
            // 调试输出
            if (entity == 1) {
                float centerX = transform.position.x + offsetX;
                float centerY = transform.position.y + offsetY;
                std::cout << "[Attack] Player facing=(" << transform.facingX << ", " << transform.facingY 
                          << ") offset=(" << offsetX << ", " << offsetY 
                          << ") center=(" << centerX << ", " << centerY << ")\n";
            }
            
            lifetimes.add(hitboxEntity, {
                .timeLeft = 0.15f,  // Hitbox 存活 0.15 秒
                .autoDestroy = true
            });
            
            attackState.hitActivated = true;
            
            std::cout << "[AttackSystem] Attack started! Duration=0.15s\n";
        }
    }
};

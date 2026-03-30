#pragma once
#include "../core/Component.h"
#include "../core/ECS.h"
#include "../components/StateMachine.h"
#include "../components/InputCommand.h"
#include "../components/DamageTag.h"
#include "../components/AttackState.h"

/**
 * @brief 状态机系统
 * 
 * 职责：
 * - 读取 InputCommand 和 DamageTag
 * - 切换 StateMachineComponent.currentState
 * - 受伤时强制切换为 Hurt 状态
 * - 进入/离开 Attack 状态时添加/移除 AttackStateComponent
 * 
 * ⚠️ Bug 1 修复：使用 AttackStateComponent 做单次触发锁
 * ⚠️ Bug 2 修复：StateMachineSystem 对 DamageTag 只读，不销毁
 */
class StateMachineSystem {
public:
    void update(
        ComponentStore<StateMachineComponent>& states,
        ComponentStore<AttackStateComponent>& attackStates,
        const ComponentStore<InputCommand>& inputs,
        const ComponentStore<DamageTag>& damageTags,
        ECS& ecs,
        float dt)
    {
        (void)dt;
        
        auto entities = states.entityList();
        for (Entity entity : entities) {
            auto& state = states.get(entity);
            
            // 优先检查 DamageTag（受击）
            if (damageTags.has(entity)) {
                // ← Bug 1 修复：离开 Attack 状态时移除 AttackStateComponent
                if (state.currentState == CharacterState::Attack) {
                    ecs.removeComponent<AttackStateComponent>(entity, attackStates);
                }
                
                state.currentState = CharacterState::Hurt;
                state.previousState = CharacterState::Hurt;
                state.stateTimer = 0.5f;
                continue;
            }
            
            // 状态计时和恢复
            if (state.stateTimer > 0.0f) {
                state.stateTimer -= dt;
                
                // Hurt 状态结束
                if (state.stateTimer <= 0.0f && state.currentState == CharacterState::Hurt) {
                    state.currentState = CharacterState::Idle;
                }
                
                // ← Bug 1 修复：Attack 状态结束，移除 AttackStateComponent
                if (state.stateTimer <= 0.0f && state.currentState == CharacterState::Attack) {
                    state.currentState = CharacterState::Idle;
                    ecs.removeComponent<AttackStateComponent>(entity, attackStates);
                }
            }
            
            // ← 架构升级：根据 moveDir 向量判断状态
            Vec2 moveDir = inputs.has(entity) ? inputs.get(entity).moveDir : Vec2{0.0f, 0.0f};
            bool attackPressed = inputs.has(entity) ? inputs.get(entity).attackPressed : false;
            
            // 优先处理攻击输入
            if (attackPressed && state.currentState != CharacterState::Attack &&
                state.currentState != CharacterState::Hurt && state.currentState != CharacterState::Dead) {
                state.currentState = CharacterState::Attack;
                state.previousState = CharacterState::Attack;
                state.stateTimer = 0.3f;
                
                // ← Bug 1 修复：切入 Attack 状态时添加 AttackStateComponent
                ecs.addComponent<AttackStateComponent>(entity, attackStates, {
                    .hitTimer = 0.0f,
                    .hitDuration = 0.3f,
                    .hitActivated = false  // ← 关键：未激活锁
                });
                
                // ← Bug 1 修复：重置攻击标志，防止持续触发
                // 注意：只在进入 Attack 状态时重置一次
                if (inputs.has(entity)) {
                    const_cast<InputCommand&>(inputs.get(entity)).attackPressed = false;
                }
                continue;
            }
            
            CharacterState newState = decideState(state, moveDir);
            
            if (newState != state.currentState) {
                state.currentState = newState;
                state.previousState = newState;
                state.stateTimer = 0.0f;
                
                // ← Bug 1 修复：离开 Attack 状态时移除 AttackStateComponent
                if (state.currentState != CharacterState::Attack) {
                    ecs.removeComponent<AttackStateComponent>(entity, attackStates);
                }
            }
        }
    }
    
private:
    // ← 架构升级：根据 Vec2 向量判断状态
    CharacterState decideState(const StateMachineComponent& state, const Vec2& moveDir) const {
        // Hurt 和 Dead 状态不可打断
        if (state.currentState == CharacterState::Hurt || 
            state.currentState == CharacterState::Dead) {
            return state.currentState;
        }
        
        // ← 架构升级：检查向量是否为 {0, 0}
        if (moveDir.x != 0.0f || moveDir.y != 0.0f) {
            return CharacterState::Move;
        }
        return CharacterState::Idle;
    }
};

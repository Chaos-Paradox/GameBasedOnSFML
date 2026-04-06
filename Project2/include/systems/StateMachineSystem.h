#pragma once
#include "../core/Component.h"
#include "../core/ECS.h"
#include "../components/StateMachine.h"
#include "../components/InputCommand.h"
#include "../components/DamageEventComponent.h"
#include "../components/AttackState.h"

/**
 * @brief 状态机系统
 * 
 * ⚠️ 严格架构：
 * 1. 获取组件数据
 * 2. 倒计时 attackBufferTimer（限时输入缓存）
 * 3. 判定当前状态的可打断性 (Cancel Window)
 * 4. 严格优先级的指令消费树 (Token Consumption)
 * 5. 只在成功切入 Attack 时清零 attackBufferTimer（精准消费）
 */
class StateMachineSystem {
public:
    void update(
        ComponentStore<StateMachineComponent>& states,
        ComponentStore<AttackStateComponent>& attackStates,
        ComponentStore<InputCommand>& inputs,
        const ComponentStore<DamageEventComponent>& damageEvents,
        ECS& ecs,
        float dt)
    {
        (void)dt;
        
        auto entities = states.entityList();
        for (Entity entity : entities) {
            // ========== 1. 获取组件数据 ==========
            auto& state = states.get(entity);
            
            // 获取输入（如果没有则创建临时对象）
            InputCommand input{Vec2{0.0f, 0.0f}, 0.0f};
            if (inputs.has(entity)) {
                input = inputs.get(entity);
            }
            
            // ========== 2. 倒计时 attackBufferTimer（限时输入缓存） ==========
            if (inputs.has(entity) && input.attackBufferTimer > 0.0f) {
                input.attackBufferTimer -= dt;
                if (input.attackBufferTimer < 0.0f) {
                    input.attackBufferTimer = 0.0f;
                }
                inputs.get(entity).attackBufferTimer = input.attackBufferTimer;
            }
            
            // ========== 3. 判定当前状态的可打断性 (Cancel Window) ==========
            bool canBeInterrupted = true;
            bool canCancelAttack = false;
            
            // 不可打断状态
            if (state.currentState == CharacterState::Dash ||
                state.currentState == CharacterState::Hurt ||
                state.currentState == CharacterState::Dead) {
                canBeInterrupted = false;
            }
            
            // Attack 状态的可打断窗口（Hitbox 已创建且过了 0.05 秒）
            if (state.currentState == CharacterState::Attack) {
                const bool hasAttackState = attackStates.has(entity);
                if (hasAttackState) {
                    const auto& attackState = attackStates.get(entity);
                    if (attackState.hitActivated && attackState.hitTimer <= 0.1f) {
                        canCancelAttack = true;  // 允许移动取消后摇
                    }
                }
            }
            
            // 不可打断状态直接跳过
            if (!canBeInterrupted) {
                continue;
            }
            
            // ========== 4. 严格优先级的指令消费树 (Token Consumption) ==========
            
            // --- 最高优先级：受伤事件（强制打断）---
            bool isHit = false;
            auto eventEntities = damageEvents.entityList();
            for (Entity eventEntity : eventEntities) {
                const auto& event = damageEvents.get(eventEntity);
                if (event.target == entity) {
                    isHit = true;
                    break;
                }
            }
            
            if (isHit) {
                state.currentState = CharacterState::Hurt;
                state.previousState = CharacterState::Hurt;
                state.stateTimer = 0.5f;
                continue;
            }
            
            // --- 状态计时和恢复 ---
            if (state.stateTimer > 0.0f) {
                state.stateTimer -= dt;
                
                if (state.stateTimer <= 0.0f && state.currentState == CharacterState::Hurt) {
                    state.currentState = CharacterState::Idle;
                    state.previousState = CharacterState::Idle;
                }
            }
            
            // --- Attack 状态处理（可取消窗口）---
            if (state.currentState == CharacterState::Attack) {
                const bool hasAttackState = attackStates.has(entity);
                
                // 攻击时间到，自动释放回 Idle
                // ← 【核心修复 2】禁止手动清零 attackBufferTimer，让时间自然流逝过期
                if (hasAttackState && attackStates.get(entity).hitTimer <= 0.0f) {
                    state.currentState = CharacterState::Idle;
                    state.previousState = CharacterState::Idle;
                    
                    // ← 禁止在这里清零 attackBufferTimer！
                    // inputs.get(entity).attackBufferTimer = 0.0f;  ❌ 错误！
                    
                    continue;
                }
                
                // 可取消窗口：移动指令抢占状态
                // ← 【核心修复 2】禁止手动清零 attackBufferTimer，让时间自然流逝过期
                if (canCancelAttack && (input.moveDir.x != 0.0f || input.moveDir.y != 0.0f)) {
                    state.currentState = CharacterState::Move;
                    state.previousState = CharacterState::Move;
                    
                    // ← 禁止在这里清零 attackBufferTimer！
                    // inputs.get(entity).attackBufferTimer = 0.0f;  ❌ 错误！
                    
                    continue;
                }
                
                // 攻击进行中，不处理其他输入
                continue;
            }
            
            // --- 攻击指令（最高优先级）---
            // ← 【核心修复 1】精准消费：只有成功切入 Attack 时才清零 attackBufferTimer
            if (input.attackBufferTimer > 0.0f) {
                state.currentState = CharacterState::Attack;
                state.previousState = CharacterState::Attack;
                
                // 初始化攻击状态组件
                ecs.addComponent<AttackStateComponent>(entity, attackStates, {
                    .hitTimer = 0.15f,
                    .hitDuration = 0.15f,
                    .hitActivated = false
                });
                
                // ← 【核心修复 1】精准消费：清零 attackBufferTimer
                if (inputs.has(entity)) {
                    inputs.get(entity).attackBufferTimer = 0.0f;
                }
                
                continue;
            }
            
            // --- 移动指令（次级优先级）---
            if (input.moveDir.x != 0.0f || input.moveDir.y != 0.0f) {
                state.currentState = CharacterState::Move;
                state.previousState = CharacterState::Move;
            }
            // --- 待机（最低优先级）---
            else {
                state.currentState = CharacterState::Idle;
                state.previousState = CharacterState::Idle;
            }
        }
    }
};

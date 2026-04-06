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
 * ⚠️ 工业级架构 - 单轨覆盖指令槽：
 * 1. 获取组件数据
 * 2. 【时间静止魔法】僵直期间暂停 intentTimer 倒计时
 * 3. 判定当前状态的可打断性 (Cancel Window)
 * 4. 单轨意图消费（Last-In-Wins，精准消费）
 * 5. 只在成功切入时清零 pendingIntent（精准消费）
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
        auto entities = states.entityList();
        for (Entity entity : entities) {
            // ========== 1. 获取组件数据 ==========
            auto& state = states.get(entity);
            
            // 获取输入（如果没有则创建临时对象）
            InputCommand input{Vec2{0.0f, 0.0f}, ActionIntent::None, 0.0f};
            if (inputs.has(entity)) {
                input = inputs.get(entity);
            }
            
            // ========== 2. 【时间静止魔法】僵直期间暂停 intentTimer 倒计时 ==========
            // 核心设计：当玩家处于被控状态时，指令保质期永远冻结
            // 这样即使玩家在挨打第 0.1 秒按了冲刺，硬直 1.0 秒结束后依然有效
            if (inputs.has(entity) && input.intentTimer > 0.0f) {
                // 只在可行动状态下倒计时（Hurt/Dead/Dash 期间暂停）
                if (state.currentState != CharacterState::Hurt &&
                    state.currentState != CharacterState::Dead &&
                    state.currentState != CharacterState::Dash) {
                    input.intentTimer -= dt;
                    if (input.intentTimer < 0.0f) {
                        input.intentTimer = 0.0f;
                    }
                }
                inputs.get(entity).intentTimer = input.intentTimer;
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
            
            // ========== 4. 单轨意图消费（Last-In-Wins） ==========
            
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
                if (hasAttackState && attackStates.get(entity).hitTimer <= 0.0f) {
                    state.currentState = CharacterState::Idle;
                    state.previousState = CharacterState::Idle;
                    continue;
                }
                
                // 可取消窗口：移动指令抢占状态
                if (canCancelAttack && (input.moveDir.x != 0.0f || input.moveDir.y != 0.0f)) {
                    state.currentState = CharacterState::Move;
                    state.previousState = CharacterState::Move;
                    continue;
                }
                
                // 攻击进行中，不处理其他输入
                continue;
            }
            
            // --- 单轨意图消费（最高优先级）---
            // 核心设计：pendingIntent 是唯一指令源，Attack/Dash 共享同一通道
            // 精准消费：只有成功切入时才清零，让时间自然流逝过期
            if (input.pendingIntent == ActionIntent::Attack && input.intentTimer > 0.0f) {
                state.currentState = CharacterState::Attack;
                state.previousState = CharacterState::Attack;
                
                // 初始化攻击状态组件
                ecs.addComponent<AttackStateComponent>(entity, attackStates, {
                    .hitTimer = 0.15f,
                    .hitDuration = 0.15f,
                    .hitActivated = false
                });
                
                // 精准消费：清零意图
                if (inputs.has(entity)) {
                    inputs.get(entity).pendingIntent = ActionIntent::None;
                    inputs.get(entity).intentTimer = 0.0f;
                }
                
                continue;
            }
            
            // --- Dash 意图消费（与 Attack 共享单轨）---
            // 注意：DashSystem 会在更早阶段消费 dashPressedSignal
            // 这里保留用于未来的 Dash 指令缓存扩展
            
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

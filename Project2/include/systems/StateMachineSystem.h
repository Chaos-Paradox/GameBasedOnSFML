#pragma once
#include "core/GameWorld.h"
#include "components/InputCommand.h"
#include "components/DamageEventComponent.h"
#include "components/AttackState.h"

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
    void update(GameWorld& world, float dt)
    {
        auto entities = world.states.entityList();
        for (Entity entity : entities) {
            // ========== 1. 获取组件数据 ==========
            auto& state = world.states.get(entity);

            // 获取输入（如果没有则创建临时对象）
            InputCommand input{Vec2{0.0f, 0.0f}, ActionIntent::None, 0.0f};
            if (world.inputs.has(entity)) {
                input = world.inputs.get(entity);
            }

            // ========== 2. 【时间静止魔法】僵直期间暂停 intentTimer 倒计时 ==========
            if (world.inputs.has(entity) && input.intentTimer > 0.0f) {
                if (state.currentState != CharacterState::Hurt &&
                    state.currentState != CharacterState::Dead &&
                    state.currentState != CharacterState::Dash &&
                    state.currentState != CharacterState::Recovery) {
                    input.intentTimer -= dt;
                    if (input.intentTimer < 0.0f) {
                        input.intentTimer = 0.0f;
                    }
                }
                world.inputs.get(entity).intentTimer = input.intentTimer;
            }

            // ========== 3. 判定当前状态的可打断性 (Cancel Window) ==========
            bool canBeInterrupted = true;
            bool canCancelAttack = false;

            if (state.currentState == CharacterState::Dash ||
                state.currentState == CharacterState::Hurt ||
                state.currentState == CharacterState::Dead ||
                state.currentState == CharacterState::KnockedAirborne ||
                state.currentState == CharacterState::Recovery) {
                canBeInterrupted = false;
            }

            if (state.currentState == CharacterState::Attack) {
                const bool hasAttackState = world.attackStates.has(entity);
                if (hasAttackState) {
                    const auto& attackState = world.attackStates.get(entity);
                    if (attackState.hasFiredDamage && attackState.hitTimer <= 0.1f) {
                        canCancelAttack = true;
                    }
                }
            }

            if (!canBeInterrupted) {
                continue;
            }

            // ========== 4. 单轨意图消费（Last-In-Wins） ==========

            // --- 最高优先级：受伤事件（强制打断）---
            bool isHit = false;
            auto eventEntities = world.damageEvents.entityList();
            for (Entity eventEntity : eventEntities) {
                const auto& event = world.damageEvents.get(eventEntity);
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
                const bool hasAttackState = world.attackStates.has(entity);

                if (hasAttackState && world.attackStates.get(entity).hitTimer <= 0.0f) {
                    state.currentState = CharacterState::Idle;
                    state.previousState = CharacterState::Idle;
                    continue;
                }

                if (canCancelAttack && (input.moveDir.x != 0.0f || input.moveDir.y != 0.0f)) {
                    state.currentState = CharacterState::Move;
                    state.previousState = CharacterState::Move;
                    continue;
                }

                continue;
            }

            // --- 单轨意图消费 ---
            if (input.pendingIntent == ActionIntent::Attack && input.intentTimer > 0.0f) {
                bool hasExistingAttack = world.attackStates.has(entity);
                if (hasExistingAttack) {
                    std::cout << "[StateMachine] ⚠️ 玩家已有 AttackState！强制覆盖 (hitTimer=" << world.attackStates.get(entity).hitTimer << ")\n";
                }

                state.currentState = CharacterState::Attack;
                state.previousState = CharacterState::Attack;

                AttackStateComponent newAttackState;
                newAttackState.hitTimer = 0.15f;
                newAttackState.hitDuration = 0.15f;
                newAttackState.hasFiredDamage = false;
                world.ecs.addComponent<AttackStateComponent>(entity, world.attackStates, newAttackState);

                std::cout << "[StateMachine] 🗡️ 攻击意图消费！pendingIntent=Attack\n";

                if (world.inputs.has(entity)) {
                    world.inputs.get(entity).pendingIntent = ActionIntent::None;
                    world.inputs.get(entity).intentTimer = 0.0f;
                }

                continue;
            }

            // --- 移动指令 ---
            if (input.moveDir.x != 0.0f || input.moveDir.y != 0.0f) {
                state.currentState = CharacterState::Move;
                state.previousState = CharacterState::Move;
            } else {
                state.currentState = CharacterState::Idle;
                state.previousState = CharacterState::Idle;
            }
        }
    }
};

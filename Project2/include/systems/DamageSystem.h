#pragma once
#include "core/GameWorld.h"
#include "core/GameJuice.h"
#include <iostream>
#include <cstdlib>

/**
 * @brief 伤害结算系统
 *
 * ⚠️ 重构（ECS 纯净原则）：
 * - 不再从 world.damageEvents（ECS 组件存储）读取，改为从 world.events.damageEvents（事件队列）读取
 * - 伤害飘字不再直接创建 LifetimeComponent，改为写入 DamageTextEvent
 */
class DamageSystem {
public:
    void update(GameWorld& world, float dt)
    {
        (void)dt;

        // 消费事件队列（不修改 ECS）
        auto& dmgEvents = world.events.damageEvents;
        for (const auto& event : dmgEvents) {
            if (event.target == INVALID_ENTITY) continue;

            if (world.characters.has(event.target)) {
                if (isInvincible(event.target, world)) {
                    std::cout << "[Combat] Dodge! Damage avoided during i-frames.\n";
                    continue;
                }

                auto& character = world.characters.get(event.target);
                int oldHP = character.currentHP;
                character.currentHP -= event.actualDamage;

                if (character.currentHP <= 0) {
                    character.currentHP = 0;
                    if (!world.deathTags.has(event.target)) {
                        world.deathTags.add(event.target, {});
                        std::cout << "[DamageSystem] DeathTag added to Entity " << event.target << "\n";
                    }
                }

                std::cout << "[DamageSystem] Entity " << event.target
                          << " took " << event.actualDamage << " damage"
                          << (event.isCritical ? " [CRITICAL!]" : "")
                          << " (HP: " << oldHP << " -> " << character.currentHP << ")\n";

                // 写入伤害飘字事件（不直接创建 ECS 实体）
                world.events.damageTextEvents.push_back({
                    .damage = event.actualDamage,
                    .isCritical = event.isCritical,
                    .position = {event.hitPosition.x, event.hitPosition.y - 40.0f}
                });

                // 处理击飞效果
                if ((event.knockbackXY > 0.0f || event.knockbackZ > 0.0f) &&
                    world.states.has(event.target) &&
                    world.states.get(event.target).currentState != CharacterState::Dead) {

                    auto& state = world.states.get(event.target);

                    if (state.currentState != CharacterState::KnockedAirborne) {
                        state.currentState = CharacterState::KnockedAirborne;
                        state.previousState = CharacterState::KnockedAirborne;
                        std::cout << "[DamageSystem] 💨 击飞！Entity " << event.target
                                  << " knockbackXY=" << event.knockbackXY
                                  << " knockbackZ=" << event.knockbackZ << "\n";
                    }

                    if (world.transforms.has(event.target)) {
                        auto& transform = world.transforms.get(event.target);

                        if (event.knockbackXY > 0.0f) {
                            transform.velocity.x = event.hitDirection.x * event.knockbackXY;
                            transform.velocity.y = event.hitDirection.y * event.knockbackXY;
                        }

                        if (event.knockbackZ > 0.0f && world.zTransforms.has(event.target)) {
                            auto& zTrans = world.zTransforms.get(event.target);
                            zTrans.vz = event.knockbackZ;
                            zTrans.z += 1.0f;
                        }
                    }
                }

                // GameJuice 打击感触发
                if (event.knockbackXY > 1000.0f) {
                    world.juice.hitStopTimer = 0.12f;
                    world.juice.timeScale = 0.0f;
                    world.juice.shakeTimer = 0.3f;
                    world.juice.shakeIntensity = 20.0f;
                    std::cout << "[GameJuice] 💥 HEAVY hit-stop + shake!\n";
                } else if (event.actualDamage > 0) {
                    world.juice.hitStopTimer = 0.06f;
                    world.juice.timeScale = 0.0f;
                    world.juice.shakeTimer = 0.15f;
                    world.juice.shakeIntensity = 8.0f;
                    std::cout << "[GameJuice] ⚡ Light hit-stop + shake!\n";
                }
            }
        }

        // 消费完毕，清空事件队列
        dmgEvents.clear();
    }

private:
    bool isInvincible(Entity entity, GameWorld& world)
    {
        if (world.states.has(entity)) {
            const auto& state = world.states.get(entity);
            if (state.currentState == CharacterState::Dash) {
                if (world.dashes.has(entity)) {
                    const auto& dash = world.dashes.get(entity);
                    if (dash.isInvincible && dash.iframeTimer > 0.0f) {
                        return true;
                    }
                }
            }
        }
        return false;
    }
};

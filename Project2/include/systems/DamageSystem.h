#pragma once
#include "core/GameWorld.h"
#include "core/GameJuice.h"
#include <iostream>
#include <cstdlib>

/**
 * @brief 伤害结算系统
 */
class DamageSystem {
public:
    void update(GameWorld& world, float dt)
    {
        (void)dt;
        auto entities = world.damageEvents.entityList();
        for (Entity eventEntity : entities) {
            const auto& event = world.damageEvents.get(eventEntity);

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

                // 生成伤害飘字实体
                Entity textEntity = world.ecs.create();
                float randomOffsetX = (static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX)) * 20.0f - 10.0f;

                world.transforms.add(textEntity, {
                    .position = {event.hitPosition.x + randomOffsetX, event.hitPosition.y - 40.0f},
                    .scale = {1.0f, 1.0f},
                    .rotation = 0.0f,
                    .velocity = {0.0f, 0.0f},
                    .facingX = 1.0f,
                    .facingY = 0.0f
                });

                world.damageTexts.add(textEntity, {
                    .text = std::to_string(event.actualDamage) + (event.isCritical ? "!" : ""),
                    .timer = 1.0f,
                    .position = {event.hitPosition.x + randomOffsetX, event.hitPosition.y - 40.0f},
                    .velocity = {0.0f, -50.0f},
                    .isCritical = event.isCritical,
                    .alpha = 1.0f,
                    .fontSize = event.isCritical ? 32.0f : 24.0f,
                    .fadeOutStart = 0.5f
                });

                world.lifetimes.add(textEntity, {
                    .timeLeft = 0.8f,
                    .autoDestroy = true
                });

                world.zTransforms.add(textEntity, {
                    .z = 50.0f,
                    .vz = 0.0f,
                    .gravity = 0.0f,
                    .height = 10.0f
                });

                std::cout << "[DamageSystem] 📝 Created damage text! ID=" << (uint32_t)textEntity << " dmg=" << event.actualDamage << "\n";

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
                    // HEAVY：炸弹爆炸、重击
                    world.juice.hitStopTimer = 0.12f;
                    world.juice.timeScale = 0.0f;
                    world.juice.shakeTimer = 0.3f;
                    world.juice.shakeIntensity = 20.0f;
                    std::cout << "[GameJuice] 💥 HEAVY hit-stop + shake!\n";
                } else if (event.actualDamage > 0) {
                    // LIGHT：普通攻击（提升强度让玩家能感知到）
                    world.juice.hitStopTimer = 0.06f;
                    world.juice.timeScale = 0.0f;
                    world.juice.shakeTimer = 0.15f;
                    world.juice.shakeIntensity = 8.0f;
                    std::cout << "[GameJuice] ⚡ Light hit-stop + shake!\n";
                }
            }

            world.damageEvents.remove(eventEntity);
        }
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

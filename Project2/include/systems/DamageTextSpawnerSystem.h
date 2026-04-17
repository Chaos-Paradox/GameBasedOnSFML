#pragma once
#include "core/GameWorld.h"
#include "core/EventQueue.h"
#include <string>

/**
 * @brief 伤害飘字生成系统
 *
 * ⚠️ 重构（ECS 纯净原则）：
 * - 从 world.events.damageTextEvents（事件队列）读取，不再从 ECS 读取
 */
class DamageTextSpawnerSystem {
public:
    void update(GameWorld& world, float dt)
    {
        (void)dt;

        for (const auto& event : world.events.damageTextEvents) {
            createDamageText(world, event);
        }

        // 消费完毕后清空
        world.events.damageTextEvents.clear();
    }

private:
    Entity createDamageText(GameWorld& world, const DamageTextEvent& event)
    {
        Entity textEntity = world.ecs.create();

        world.damageTexts.add(textEntity, {
            .text = std::to_string(event.damage) + (event.isCritical ? "!" : ""),
            .timer = 1.0f,
            .position = event.position,
            .velocity = {0.0f, -50.0f},
            .isCritical = event.isCritical,
            .alpha = 1.0f,
            .fontSize = event.isCritical ? 32.0f : 24.0f,
            .fadeOutStart = 0.5f
        });

        world.zTransforms.add(textEntity, {
            .z = 50.0f,
            .vz = 0.0f,
            .gravity = 0.0f,
            .height = 10.0f
        });

        return textEntity;
    }
};

#pragma once
#include "core/GameWorld.h"
#include <sstream>

/**
 * @brief 伤害飘字生成系统
 */
class DamageTextSpawnerSystem {
public:
    void update(GameWorld& world, float dt)
    {
        (void)dt;
        auto eventEntities = world.damageEvents.entityList();
        for (Entity eventEntity : eventEntities) {
            const auto& event = world.damageEvents.get(eventEntity);
            if (event.target == INVALID_ENTITY) continue;
            createDamageText(world, event);
        }
    }

private:
    Entity createDamageText(GameWorld& world, const DamageEventComponent& event)
    {
        Entity textEntity = world.ecs.create();

        std::ostringstream oss;
        oss << event.actualDamage;
        std::string damageText = oss.str();
        if (event.isCritical) damageText += "!";

        world.damageTexts.add(textEntity, {
            .text = damageText,
            .timer = 1.0f,
            .position = event.hitPosition,
            .velocity = {0.0f, -50.0f},
            .isCritical = event.isCritical,
            .alpha = 1.0f,
            .fontSize = event.isCritical ? 32.0f : 24.0f,
            .fadeOutStart = 0.5f
        });

        return textEntity;
    }
};

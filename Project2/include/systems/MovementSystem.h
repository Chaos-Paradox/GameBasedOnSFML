#pragma once
#include "../core/Component.h"
#include "../components/Transform.h"

class MovementSystem {
public:
    void update(
        ComponentStore<TransformComponent>& transforms,
        float dt)
    {
        auto entities = transforms.entityList();
        for (Entity entity : entities) {
            auto& transform = transforms.get(entity);
            transform.position.x += transform.velocity.x * dt;
            transform.position.y += transform.velocity.y * dt;
        }
    }
};

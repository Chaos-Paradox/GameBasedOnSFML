#pragma once
#include "EventBus.hpp"
#include "Entity.hpp"
#include "ComponentManager.hpp"
#include "Components.hpp"


class MovementSystem {
public:
    MovementSystem(ComponentManager& cm) : cm(cm) {}
    void update(float dt) {
        cm.forEach<VelocityComponent>([&](int id, VelocityComponent& v) {
            Entity e{ id };
            if (auto* p = cm.getComponent<PositionComponent>(e))
                p->position += v.velocity * dt;
            });
    }
private: ComponentManager& cm;
};

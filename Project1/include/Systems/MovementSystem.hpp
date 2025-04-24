#pragma once
#include "EventBus.hpp"
#include "Entity.hpp"
#include "ComponentManager.hpp"
#include "Components.hpp"

class MovementSystem {
public:
    MovementSystem(EventBus& bus, ComponentManager& cm)
        : cm(cm)
    {
        bus.subscribe<MoveEvent>([&](const MoveEvent& e) {
            if (auto* v = cm.getComponent<VelocityComponent>(e.entity)) {
                v->velocity = e.dir * 200.f;
            }
            });
    }

    void update(float dt) {
        cm.forEach<VelocityComponent>([&](int id, VelocityComponent& v) {
            Entity e{ id };
            if (auto* p = cm.getComponent<PositionComponent>(e)) {
                p->position += v.velocity * dt;
            }
            });
    }

private:
    ComponentManager& cm;
};

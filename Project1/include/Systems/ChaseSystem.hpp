#pragma once
#include "ComponentManager.hpp"
#include "Components.hpp"
#include "Entity.hpp"
#include <cmath>

class ChaseSystem {
public:
    ChaseSystem(ComponentManager& cm, Entity player)
      : cm(cm), player(player) {}

    void update(float dt) {
        auto* ppos = cm.getComponent<PositionComponent>(player);
        if (!ppos) return;

        cm.forEach<ChaseComponent>([&](int id, ChaseComponent& chase){
            Entity e{id};
            auto* pos = cm.getComponent<PositionComponent>(e);
            auto* vel = cm.getComponent<VelocityComponent>(e);
            if (!pos||!vel) return;
            sf::Vector2f delta = ppos->position - pos->position;
            float dist = std::sqrt(delta.x*delta.x + delta.y*delta.y);
            if (dist>1e-3f){
                sf::Vector2f dir = delta/dist;
                vel->velocity = dir * chase.speed;
            } else vel->velocity = {0,0};
        });
    }

private: ComponentManager& cm; Entity player;
};

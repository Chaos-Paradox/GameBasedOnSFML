#pragma once
#include <SFML/Graphics/RenderWindow.hpp>
#include "ComponentManager.hpp"
#include "Components.hpp"

class RenderSystem {
public:
    RenderSystem(sf::RenderWindow& w, ComponentManager& cm)
        : window(w), cm(cm) {}

    void update() {
        window.clear();
        // 遍历所有 SpriteComponent 实例
        cm.forEach<SpriteComponent>([&](int id, SpriteComponent& sprComp) {
            Entity e{ id };
            if (auto* pos = cm.getComponent<PositionComponent>(e)) {
                sf::Sprite sprite(sprComp.tex);
                sprite.setPosition(pos->position);
                if (auto* vel = cm.getComponent<VelocityComponent>(e)) {
                    if (vel->velocity.x > 0) {
                        sprComp.flipX = 1.f;
                    }
                    else if (vel->velocity.x < 0) {
                        sprComp.flipX = -1.f;
                    }
                    sprite.setScale({ sprComp.flipX, 1.f });
                    //sprite.setScale(1.f, 1.f);
                }


                window.draw(sprite);
            }

            //spr->sprite.setScale(1.f, 1.f);


            });
        window.display();
    }

private:
    sf::RenderWindow& window;
    ComponentManager& cm;
};

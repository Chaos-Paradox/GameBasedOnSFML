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
                window.draw(sprite);
            }
            });
        window.display();
    }

private:
    sf::RenderWindow& window;
    ComponentManager& cm;
};

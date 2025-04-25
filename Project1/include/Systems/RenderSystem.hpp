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
                auto bounds = sprite.getLocalBounds();
                sprite.setOrigin( bounds.size / 2.f );



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

        cm.forEach<HealthComponent>([&](int id, HealthComponent& hpComp) {
            Entity e{ id };
            auto* pos = cm.getComponent<PositionComponent>(e);
            if (!pos) return;

            float maxHp = 100.f;
            float width = 40.f;
            float height = 5.f;

            sf::Vector2f barPos = pos->position + sf::Vector2f(-width / 2.f, -40.f); // 位于实体上方

            // 背景条（灰色）
            sf::RectangleShape bg(sf::Vector2f(width, height));
            bg.setFillColor(sf::Color(50, 50, 50));
            bg.setPosition(barPos);

            // 前景条（红色）
            sf::RectangleShape fg(sf::Vector2f(width * (hpComp.hp / maxHp), height));
            fg.setFillColor(sf::Color::Red);
            fg.setPosition(barPos);

            window.draw(bg);
            window.draw(fg);
            });



        window.display();
    }

private:
    sf::RenderWindow& window;
    ComponentManager& cm;
};

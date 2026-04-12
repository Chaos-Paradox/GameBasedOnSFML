#pragma once

#ifdef ENABLE_SFML

#include <SFML/Graphics.hpp>
#include "core/GameWorld.h"
#include <vector>
#include <cstdint>

/**
 * @brief 伤害飘字渲染系统
 */
class DamageTextRenderSystem {
public:
    void update(GameWorld& world, sf::RenderWindow& window, const sf::Font& font, float dt)
    {
        std::vector<Entity> entitiesToDestroy;

        auto entities = world.damageTexts.entityList();
        for (Entity entity : entities) {
            if (!world.damageTexts.has(entity)) continue;

            auto& textComp = world.damageTexts.get(entity);

            textComp.timer -= dt;
            textComp.position.x += textComp.velocity.x * dt;
            textComp.position.y += textComp.velocity.y * dt;

            if (textComp.timer < textComp.fadeOutStart) {
                textComp.alpha = textComp.timer / textComp.fadeOutStart;
            }

            renderText(window, font, textComp);

            if (textComp.timer <= 0.0f) {
                entitiesToDestroy.push_back(entity);
            }
        }

        for (Entity entity : entitiesToDestroy) {
            if (world.damageTexts.has(entity)) {
                world.damageTexts.remove(entity);
            }
            world.ecs.destroy(entity);
        }
    }

private:
    void renderText(sf::RenderWindow& window, const sf::Font& font, const DamageTextComponent& textComp)
    {
        sf::Text text(font, textComp.text, static_cast<unsigned int>(textComp.fontSize));

        if (textComp.isCritical) {
            text.setStyle(sf::Text::Style::Bold);
        }

        auto bounds = text.getLocalBounds();
        text.setOrigin({bounds.size.x / 2.0f, bounds.size.y / 2.0f});
        text.setPosition({textComp.position.x, textComp.position.y});

        uint8_t alphaValue = static_cast<uint8_t>(textComp.alpha * 255.0f);
        if (textComp.isCritical) {
            text.setFillColor(sf::Color(255, 50, 50, alphaValue));
        } else {
            text.setFillColor(sf::Color(255, 255, 255, alphaValue));
        }

        text.setOutlineColor(sf::Color(0, 0, 0, alphaValue));
        text.setOutlineThickness(2.0f);

        window.draw(text);
    }
};

#else

#include "core/GameWorld.h"

class DamageTextRenderSystem {
public:
    template<typename... Args>
    void update(Args&&...) const {}
};

#endif  // ENABLE_SFML

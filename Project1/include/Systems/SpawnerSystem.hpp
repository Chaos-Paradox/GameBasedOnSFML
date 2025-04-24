#pragma once
#include "SpawnConfig.hpp"
#include "EntityManager.hpp"
#include "ComponentManager.hpp"
#include "Components.hpp"
#include <random>
#include <SFML/Graphics/RenderWindow.hpp>

class SpawnerSystem {
public:
    SpawnerSystem(EntityManager& em, ComponentManager& cm, Entity player,
        const sf::RenderWindow& window)
        : em(em), cm(cm), player(player), window(window),
        rng(std::random_device{}()), angleDist(0, 2 * 3.14159f) {}

    void addConfig(SpawnConfig cfg) { configs.push_back(std::move(cfg)); }

    void update() {
        float now = clock.getElapsedTime().asSeconds();
        if (now < interval) return;
        clock.restart();

        auto* ppos = cm.getComponent<PositionComponent>(player);
        if (!ppos) return;

        for (auto& cfg : configs) {
            float angle = angleDist(rng);
            std::uniform_real_distribution<float> rd(cfg.minRadius, cfg.maxRadius);
            float r = rd(rng);
            sf::Vector2f pos = {
                ppos->position.x + std::cos(angle) * r,
                ppos->position.y + std::sin(angle) * r
            };
            Entity e = em.create();
            cm.addComponent<PositionComponent>(e, PositionComponent{ pos });
            cm.addComponent<VelocityComponent>(e, VelocityComponent{ {0,0} });
            cfg.initializer(e);
        }
    }

private:
    EntityManager& em; ComponentManager& cm; Entity player;
    const sf::RenderWindow& window;
    std::vector<SpawnConfig> configs;
    std::mt19937 rng;
    std::uniform_real_distribution<float> angleDist;
    sf::Clock clock; float interval = 5.f;
};

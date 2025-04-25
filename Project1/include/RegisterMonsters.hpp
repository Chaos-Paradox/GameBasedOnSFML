// RegisterMonsters.cpp
#include "EntityFactory.hpp"
#include "Components.hpp"

void registerMonsters() {
    auto& factory = EntityFactory::instance();

    factory.registerType("bat", [](EntityManager& em, ComponentManager& cm, Entity e, const sf::Vector2f&) {
        cm.addComponent<SpriteComponent>(e, SpriteComponent{ "./material/pictures/bat.png" });
        cm.addComponent<ChasePlayerTag>(e, {});
        cm.addComponent<MonsterTag>(e, {});
        cm.addComponent<HealthComponent>(e, { 30, 30 });
        cm.addComponent<DamageFlashComponent>(e, {});
    });

    factory.registerType("duck", [](EntityManager& em, ComponentManager& cm, Entity e, const sf::Vector2f&) {
        cm.addComponent<SpriteComponent>(e, SpriteComponent{ "./material/pictures/duck.png" });
        cm.addComponent<ChasePlayerTag>(e, {});
        cm.addComponent<MonsterTag>(e, {});
        cm.addComponent<HealthComponent>(e, { 50, 50 });
        cm.addComponent<DamageFlashComponent>(e, {});
    });
}

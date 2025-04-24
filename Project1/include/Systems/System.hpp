#pragma once
#include "Component.hpp"


class WeaponSystem {
public:
    WeaponSystem(EntityManager& em, ComponentManager& cm)
        : entityManager(em), componentManager(cm) {}

    void shoot(Entity shooter, const sf::Vector2f& direction) {
        auto* pos = componentManager.getComponent<PositionComponent>(shooter);
        if (!pos) return;

        Entity bullet = entityManager.createEntity();

        sf::Vector2f velocity = normalize(direction) * 600.0f;

        componentManager.addComponent(bullet, PositionComponent{ pos->position });
        componentManager.addComponent(bullet, VelocityComponent{ velocity });
        componentManager.addComponent(bullet, DamageComponent{ 10.0f });
        componentManager.addComponent(bullet, LifetimeComponent{ 3.0f });
        componentManager.addComponent(bullet, CollisionComponent{ 4.0f, true });
        componentManager.addComponent(bullet, BulletTagComponent{});

        // 渲染组件（可选）
        componentManager.addComponent(bullet, SpriteComponent{ bulletSprite });
    }

private:
    EntityManager& entityManager;
    ComponentManager& componentManager;

    sf::Vector2f normalize(sf::Vector2f v) {
        float len = std::sqrt(v.x * v.x + v.y * v.y);
        return len > 0 ? v / len : sf::Vector2f(0.f, 0.f);
    }
};


class LifetimeSystem {
public:
    LifetimeSystem(EntityManager& em, ComponentManager& cm)
        : entityManager(em), componentManager(cm) {}

    void update(float dt) {
        // 可以优化为只遍历有 LifetimeComponent 的实体
        for (int id : entityManager.activeEntities) {
            Entity e(id);
            auto* life = componentManager.getComponent<LifetimeComponent>(e);
            if (life) {
                life->timeRemaining -= dt;
                if (life->timeRemaining <= 0.0f) {
                    entityManager.destroyEntity(e);
                }
            }
        }
    }

private:
    EntityManager& entityManager;
    ComponentManager& componentManager;
};
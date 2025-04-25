#pragma once

#include "ComponentManager.hpp"
#include "Components.hpp"
#include <SFML/System.hpp>
#include <iostream>
#include <cmath>

class CombatSystem {
public:
    CombatSystem(ComponentManager& cm, Entity player)
        : cm(cm), player(player) {}

    void update() {
        auto* playerPos = cm.getComponent<PositionComponent>(player);
        auto* playerHealth = cm.getComponent<HealthComponent>(player);

        if (!playerPos || !playerHealth || playerHealth->hp <= 0) return;

        // 遍历所有怪物，检查是否碰到主角
        cm.forEach<EnemyTag>([&](int id, EnemyTag&) {
            Entity enemy{id};

            auto* enemyPos = cm.getComponent<PositionComponent>(enemy);
            if (!enemyPos) return;

            float dx = playerPos->position.x - enemyPos->position.x;
            float dy = playerPos->position.y - enemyPos->position.y;
            float distSquared = dx * dx + dy * dy;

            const float collisionRadius = 30.f;

            if (distSquared < collisionRadius * collisionRadius) {
                // 怪物碰到玩家，扣血
                playerHealth->hp = std::max(0, playerHealth->hp - 1);

                std::cout << "Player hit! HP: " << playerHealth->hp << std::endl;

                // 触发闪烁
                //if (auto* flash = cm.getComponent<DamageFlashComponent>(player)) {
                //    flash->active = true;
                //    flash->timer.restart();
                //}
                // 扣血后触发闪烁
                if (auto* df = cm.getComponent<DamageFlashComponent>(player)) {
                    df->timer = 0.5f; // 每次受伤闪烁 0.5 秒
                }

            }
        });

        // 删除所有 HP <= 0 的实体（包括玩家或怪物）
        cm.forEach<HealthComponent>([&](int id, HealthComponent& health) {
            if (health.hp <= 0) {
                Entity e{id};
                cm.destroyEntity(e);
                std::cout << "Entity " << id << " has died and was destroyed.\n";
            }
        });
    }

private:
    ComponentManager& cm;
    Entity player;
};

//class CombatSystem {
//public:
//    CombatSystem(ComponentManager& cm, Entity player)
//        : cm(cm), player(player) {}
//
//    void update() {
//        auto* playerPos = cm.getComponent<PositionComponent>(player);
//        auto* playerHealth = cm.getComponent<HealthComponent>(player);
//        auto* playerFaction = cm.getComponent<FactionComponent>(player);
//
//        if (!playerPos || !playerHealth || !playerFaction || playerHealth->hp <= 0) return;
//
//        // 遍历所有有 FactionComponent 的实体
//        cm.forEach<FactionComponent>([&](int id, FactionComponent& fc) {
//            Entity other{ id };
//            if (fc.faction != Faction::Enemy) return;
//
//            auto* enemyPos = cm.getComponent<PositionComponent>(other);
//            if (!enemyPos) return;
//
//            float dx = playerPos->position.x - enemyPos->position.x;
//            float dy = playerPos->position.y - enemyPos->position.y;
//            float distSq = dx * dx + dy * dy;
//            const float collisionRadius = 30.f;
//
//            if (distSq < collisionRadius * collisionRadius) {
//                playerHealth->hp = std::max(0, playerHealth->hp - 1);
//                std::cout << "Player hit! HP: " << playerHealth->hp << std::endl;
//
//                if (auto* df = cm.getComponent<DamageFlashComponent>(player)) {
//                    df->timer = 0.5f;
//                }
//            }
//            });
//
//        // 删除死亡实体
//        cm.forEach<HealthComponent>([&](int id, HealthComponent& health) {
//            if (health.hp <= 0) {
//                Entity e{ id };
//                cm.destroyEntity(e);
//                std::cout << "Entity " << id << " has died and was destroyed.\n";
//            }
//            });
//    }
//
//private:
//    ComponentManager& cm;
//    Entity player;
//};
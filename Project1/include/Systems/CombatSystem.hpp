#pragma once

#include "ComponentManager.hpp"
#include "Components.hpp"
#include <SFML/System.hpp>
#include <iostream>
#include <vector>
#include <cmath>

class CombatSystem {
public:
    CombatSystem(ComponentManager& cm, Entity player)
        : cm(cm), player(player) {}

    void update() {
        auto* playerPos = cm.getComponent<PositionComponent>(player);
        auto* playerHealth = cm.getComponent<HealthComponent>(player);

        if (!playerPos || !playerHealth || playerHealth->hp <= 0) return;

        // йǷ
        cm.forEach<EnemyTag>([&](int id, EnemyTag&) {
            Entity enemy{id};

            auto* enemyPos = cm.getComponent<PositionComponent>(enemy);
            if (!enemyPos) return;

            float dx = playerPos->position.x - enemyPos->position.x;
            float dy = playerPos->position.y - enemyPos->position.y;
            float distSquared = dx * dx + dy * dy;

            const float collisionRadius = 30.f;

            if (distSquared < collisionRadius * collisionRadius) {
                // ңѪ
                playerHealth->hp = std::max(0, playerHealth->hp - 1);

                std::cout << "Player hit! HP: " << playerHealth->hp << std::endl;

                // ˸
                //if (auto* flash = cm.getComponent<DamageFlashComponent>(player)) {
                //    flash->active = true;
                //    flash->timer.restart();
                //}
                // Ѫ󴥷˸
                if (auto* df = cm.getComponent<DamageFlashComponent>(player)) {
                    df->timer = 0.5f; // ÿ˸ 0.5 
                }

            }
        });

        // ɾ HP <= 0 ʵ壨һ
        // 删除 HP <= 0 的实体（收集后删除，避免迭代器失效）
        std::vector<int> deadEntities;
        cm.forEach<HealthComponent>([&](int id, HealthComponent& health) {
            if (health.hp <= 0) {
                deadEntities.push_back(id);
            }
        });
        
        // 遍历结束后再删除（排除玩家实体）
        for (int id : deadEntities) {
            if (id == player.id) {
                std::cout << "Player died! Game Over.\n";
                continue;  // 不销毁玩家
            }
            Entity e{id};
            cm.destroyEntity(e);
            std::cout << "Entity " << id << " has died and was destroyed.\n";
        }
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
//        //  FactionComponent ʵ
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
//        // ɾʵ
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
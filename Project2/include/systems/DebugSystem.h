#pragma once
#include "core/GameWorld.h"
#include <iostream>
#include <iomanip>

/**
 * @brief 调试系统
 */
class DebugSystem {
public:
    void printEntityList(GameWorld& world)
    {
        std::cout << "\n";
        std::cout << "╔══════════════════════════════════════════════════════════╗\n";
        std::cout << "║              实体资源列表 (Entity List)                  ║\n";
        std::cout << "╠══════════════════════════════════════════════════════════╣\n";

        const auto& entities = world.ecs.entities();

        if (entities.empty()) {
            std::cout << "║  [空] 没有实体                                          ║\n";
        } else {
            std::cout << "║  总实体数：" << std::setw(3) << entities.size()
                      << "                                          ║\n";
            std::cout << "╠══════════════════════════════════════════════════════════╣\n";

            for (Entity entity : entities) {
                std::cout << "║  Entity #" << std::setw(3) << entity << ": ";

                std::vector<std::string> components;

                if (world.transforms.has(entity)) {
                    const auto& t = world.transforms.get(entity);
                    components.push_back("Transform(" +
                        std::to_string((int)t.position.x) + "," +
                        std::to_string((int)t.position.y) + ")");
                }

                if (world.characters.has(entity)) {
                    const auto& c = world.characters.get(entity);
                    components.push_back("Character(HP:" +
                        std::to_string(c.currentHP) + "/" +
                        std::to_string(c.maxHP) + ")");
                }

                if (world.states.has(entity)) {
                    const auto& s = world.states.get(entity);
                    const char* stateNames[] = {"Idle", "Move", "Attack", "Hurt", "Dead"};
                    int stateIdx = static_cast<int>(s.currentState);
                    if (stateIdx >= 0 && stateIdx < 5) {
                        components.push_back(std::string("State(") + stateNames[stateIdx] + ")");
                    }
                }

                if (world.itemDatas.has(entity)) {
                    const auto& item = world.itemDatas.get(entity);
                    components.push_back("Item(ID:" +
                        std::to_string(item.itemId) +
                        ",x" + std::to_string(item.amount) + ")");
                }

                if (world.evolutions.has(entity)) {
                    const auto& evo = world.evolutions.get(entity);
                    components.push_back("Evo(" +
                        std::to_string(evo.evolutionPoints) + ")");
                }

                if (components.empty()) {
                    std::cout << "[无组件]                                  ║\n";
                } else {
                    std::string line;
                    for (size_t i = 0; i < components.size(); i++) {
                        if (i > 0) line += ", ";
                        line += components[i];
                    }
                    std::cout << std::setw(40) << std::left << line
                              << " ║\n";
                }
            }
        }

        std::cout << "╚══════════════════════════════════════════════════════════╝\n";
        std::cout << std::endl;
    }

    void printLootList(GameWorld& world)
    {
        std::cout << "\n";
        std::cout << "╔══════════════════════════════════════════════════════════╗\n";
        std::cout << "║              掉落物列表 (Loot List)                      ║\n";
        std::cout << "╠══════════════════════════════════════════════════════════╣\n";

        auto lootEntities = world.itemDatas.entityList();

        if (lootEntities.empty()) {
            std::cout << "║  [空] 没有掉落物                                        ║\n";
        } else {
            std::cout << "║  总掉落物数：" << std::setw(2) << lootEntities.size()
                      << "                                         ║\n";
            std::cout << "╠══════════════════════════════════════════════════════════╣\n";

            for (Entity loot : lootEntities) {
                std::cout << "║  Loot #" << std::setw(3) << loot << ": ";

                if (world.transforms.has(loot) && world.itemDatas.has(loot)) {
                    const auto& t = world.transforms.get(loot);
                    const auto& item = world.itemDatas.get(loot);

                    std::cout << "Pos(" << std::setw(3) << (int)t.position.x
                              << "," << std::setw(3) << (int)t.position.y << ") "
                              << "Item(ID:" << item.itemId
                              << ",x" << item.amount << ")";
                    std::cout << std::string(20, ' ') << " ║\n";
                }
            }
        }

        std::cout << "╚══════════════════════════════════════════════════════════╝\n";
        std::cout << std::endl;
    }
};

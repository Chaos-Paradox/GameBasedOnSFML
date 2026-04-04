#pragma once
#include "../core/Component.h"
#include "../core/ECS.h"
#include "../components/Transform.h"
#include "../components/Character.h"
#include "../components/StateMachine.h"
#include "../components/ItemData.h"
#include "../components/Evolution.h"
#include <iostream>
#include <iomanip>

/**
 * @brief 调试系统
 * 
 * 职责：
 * - 显示当前所有实体及其组件信息
 * - 按组件类型分类显示
 * - 支持终端输出
 * 
 * 使用方法：
 * DebugSystem debug;
 * debug.printEntityList(...);  // 打印实体列表
 */
class DebugSystem {
public:
    /**
     * @brief 打印实体列表到终端
     */
    void printEntityList(
        const ECS& ecs,
        const ComponentStore<TransformComponent>& transforms,
        const ComponentStore<CharacterComponent>& characters,
        const ComponentStore<StateMachineComponent>& states,
        const ComponentStore<ItemDataComponent>& itemDatas,
        const ComponentStore<EvolutionComponent>& evolutions)
    {
        std::cout << "\n";
        std::cout << "╔══════════════════════════════════════════════════════════╗\n";
        std::cout << "║              实体资源列表 (Entity List)                  ║\n";
        std::cout << "╠══════════════════════════════════════════════════════════╣\n";
        
        const auto& entities = ecs.entities();
        
        if (entities.empty()) {
            std::cout << "║  [空] 没有实体                                          ║\n";
        } else {
            std::cout << "║  总实体数：" << std::setw(3) << entities.size() 
                      << "                                          ║\n";
            std::cout << "╠══════════════════════════════════════════════════════════╣\n";
            
            for (Entity entity : entities) {
                std::cout << "║  Entity #" << std::setw(3) << entity << ": ";
                
                // 显示组件
                std::vector<std::string> components;
                
                if (transforms.has(entity)) {
                    const auto& t = transforms.get(entity);
                    components.push_back("Transform(" + 
                        std::to_string((int)t.position.x) + "," + 
                        std::to_string((int)t.position.y) + ")");
                }
                
                if (characters.has(entity)) {
                    const auto& c = characters.get(entity);
                    components.push_back("Character(HP:" + 
                        std::to_string(c.currentHP) + "/" + 
                        std::to_string(c.maxHP) + ")");
                }
                
                if (states.has(entity)) {
                    const auto& s = states.get(entity);
                    const char* stateNames[] = {"Idle", "Move", "Attack", "Hurt", "Dead"};
                    int stateIdx = static_cast<int>(s.currentState);
                    if (stateIdx >= 0 && stateIdx < 5) {
                        components.push_back(std::string("State(") + stateNames[stateIdx] + ")");
                    }
                }
                
                if (itemDatas.has(entity)) {
                    const auto& item = itemDatas.get(entity);
                    components.push_back("Item(ID:" + 
                        std::to_string(item.itemId) + 
                        ",x" + std::to_string(item.amount) + ")");
                }
                
                if (evolutions.has(entity)) {
                    const auto& evo = evolutions.get(entity);
                    components.push_back("Evo(" + 
                        std::to_string(evo.evolutionPoints) + ")");
                }
                
                // 打印组件列表
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
    
    /**
     * @brief 打印掉落物列表
     */
    void printLootList(
        const ComponentStore<TransformComponent>& transforms,
        const ComponentStore<ItemDataComponent>& itemDatas)
    {
        std::cout << "\n";
        std::cout << "╔══════════════════════════════════════════════════════════╗\n";
        std::cout << "║              掉落物列表 (Loot List)                      ║\n";
        std::cout << "╠══════════════════════════════════════════════════════════╣\n";
        
        auto lootEntities = itemDatas.entityList();
        
        if (lootEntities.empty()) {
            std::cout << "║  [空] 没有掉落物                                        ║\n";
        } else {
            std::cout << "║  总掉落物数：" << std::setw(2) << lootEntities.size() 
                      << "                                         ║\n";
            std::cout << "╠══════════════════════════════════════════════════════════╣\n";
            
            for (Entity loot : lootEntities) {
                std::cout << "║  Loot #" << std::setw(3) << loot << ": ";
                
                if (transforms.has(loot) && itemDatas.has(loot)) {
                    const auto& t = transforms.get(loot);
                    const auto& item = itemDatas.get(loot);
                    
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

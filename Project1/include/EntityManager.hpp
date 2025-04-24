
#pragma once
#include <unordered_set>
#include "Entity.hpp"

class ComponentManager; // ✅ 前向声明

//class EntityManager {
//public:
//    EntityManager(ComponentManager& cm) : componentManager(cm) {}
//
//    Entity createEntity() {
//        Entity entity(nextId++);
//        activeEntities.insert(entity.id);
//        return entity;
//    }
//
//    void destroyEntity(Entity entity) {
//        activeEntities.erase(entity.id);
//        componentManager.removeAllComponents(entity); // ✅ 组件移除委托出去
//    }
//
//    bool isAlive(Entity entity) {
//        return activeEntities.count(entity.id) > 0;
//    }
//
//    std::unordered_set<int> activeEntities;
//
//private:
//    int nextId = 0;
//    ComponentManager& componentManager;  // ✅ 持有引用或指针（解耦）
//};

class EntityManager {
public:
    EntityManager(ComponentManager& cm) : cm(cm) {}
    Entity create() {
        Entity e{ nextId++ }; alive.insert(e.id); return e;
    }
    bool exists(Entity e) { return alive.count(e.id); }
private:
    int nextId = 0;
    std::unordered_set<int> alive;
    ComponentManager& cm;
};
#pragma once
#include <unordered_set>


class Entity {
public:
    int id;
    Entity(int id) : id(id) {}
    bool operator==(const Entity& other) const { return id == other.id; }
};

//class EntityManager {
//public:
//    Entity createEntity();
//    void destroyEntity(Entity entity);
//    bool isAlive(Entity entity);
//
//    void setComponentManager(ComponentManager* cm) {
//        componentManager = cm;
//    }
//
//private:
//    int nextId = 0;
//    std::unordered_set<int> activeEntities;
//    ComponentManager* componentManager = nullptr;
//};

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

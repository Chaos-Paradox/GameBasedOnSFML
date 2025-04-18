#pragma once
#include <unordered_set>

class ComponentManager; // ✅ 前向声明

class Entity {
public:
    int id;
    Entity(int id) : id(id) {}
    bool operator==(const Entity& other) const { return id == other.id; }
};

class EntityManager {
public:
    Entity createEntity();
    void destroyEntity(Entity entity);
    bool isAlive(Entity entity);

    void setComponentManager(ComponentManager* cm) {
        componentManager = cm;
    }

private:
    int nextId = 0;
    std::unordered_set<int> activeEntities;
    ComponentManager* componentManager = nullptr;
};

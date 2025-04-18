#pragma once
#include <unordered_map>
#include "Components.hpp"
#include "Entity.hpp"

class ComponentManager {
public:
    template<typename T>
    void addComponent(Entity entity, const T& component) {
        getComponentMap<T>()[entity.id] = component;
    }

    template<typename T>
    void removeComponent(Entity entity) {
        getComponentMap<T>().erase(entity.id);
    }

    template<typename T>
    T* getComponent(Entity entity) {
        auto& map = getComponentMap<T>();
        auto it = map.find(entity.id);
        if (it != map.end()) return &it->second;
        return nullptr;
    }

    template<typename T>
    bool hasComponent(Entity entity) {
        return getComponentMap<T>().count(entity.id) > 0;
    }

    void removeAllComponents(Entity entity) {
        removeComponent<PositionComponent>(entity);
        removeComponent<VelocityComponent>(entity);
        removeComponent<CollisionComponent>(entity);
    }

private:
    template<typename T>
    std::unordered_map<int, T>& getComponentMap() {
        static std::unordered_map<int, T> componentMap;
        return componentMap;
    }
};

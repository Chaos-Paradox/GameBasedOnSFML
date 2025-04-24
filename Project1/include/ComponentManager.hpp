#pragma once
#include <unordered_map>
#include "Components.hpp"
#include "Entity.hpp"

//class ComponentManager {
//public:
//    ComponentManager() = default; // ✅ 显式构造函数，当前无需额外初始化
//
//    template<typename T>
//    void addComponent(Entity entity, const T& component) {
//        getComponentMap<T>()[entity.id] = component;
//    }
//
//    template<typename T>
//    void removeComponent(Entity entity) {
//        getComponentMap<T>().erase(entity.id);
//    }
//
//    template<typename T>
//    T* getComponent(Entity entity) {
//        auto& map = getComponentMap<T>();
//        auto it = map.find(entity.id);
//        if (it != map.end()) return &it->second;
//        return nullptr;
//    }
//
//    template<typename T>
//    bool hasComponent(Entity entity) {
//        return getComponentMap<T>().count(entity.id) > 0;
//    }
//
//    void removeAllComponents(Entity entity) {
//        removeComponent<PositionComponent>(entity);
//        removeComponent<VelocityComponent>(entity);
//        removeComponent<CollisionComponent>(entity);
//    }
//
//private:
//    template<typename T>
//    std::unordered_map<int, T>& getComponentMap() {
//        static std::unordered_map<int, T> componentMap;
//        return componentMap;
//    }
//};



template<typename T>
using Map = std::unordered_map<int, T>;

class ComponentManager {
public:
    template<typename T>
    void addComponent(Entity e, T&& comp) {
        get<T>()[e.id] = std::forward<T>(comp);
    }

    template<typename T>
    T* getComponent(Entity e) {
        auto& m = get<T>();
        auto it = m.find(e.id);
        return it == m.end() ? nullptr : &it->second;
    }

    template<typename T>
    void forEach(std::function<void(int, T&)> func) {
        for (auto& [id, comp] : get<T>()) {
            func(id, comp);
        }
    }

private:
    template<typename T>
    Map<T>& get() {
        static Map<T> m;
        return m;
    }
};

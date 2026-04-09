#pragma once
#include <unordered_map>
#include <functional>
#include "Components.hpp"
#include "Entity.hpp"

template<typename T>
using Map = std::unordered_map<int, T>;

class ComponentManager {
public:
    // 添加组件
    template<typename T>
    void addComponent(Entity e, T&& comp) {
        get<T>()[e.id] = std::forward<T>(comp);
    }

    // 获取组件
    template<typename T>
    T* getComponent(Entity e) {
        auto& m = get<T>();
        auto it = m.find(e.id);
        return it == m.end() ? nullptr : &it->second;
    }

    // 遍历所有组件
    template<typename T>
    void forEach(std::function<void(int, T&)> func) {
        for (auto& [id, comp] : get<T>()) {
            func(id, comp);
        }
    }

    // 删除与实体相关的所有组件
    void destroyEntity(Entity e) {
        // 遍历所有组件类型并删除该实体的相关数据
        destroyComponent<PositionComponent>(e);
        destroyComponent<VelocityComponent>(e);
        destroyComponent<SpriteComponent>(e);
        destroyComponent<HealthComponent>(e);
        destroyComponent<DamageFlashComponent>(e);
        destroyComponent<EnemyTag>(e);
        // 其他组件类型也可以继续添加
    }

private:
    // 获取特定类型的组件容器
    template<typename T>
    Map<T>& get() {
        static Map<T> m;
        return m;
    }

    // 删除某个组件类型的某个实体的数据
    template<typename T>
    void destroyComponent(Entity e) {
        auto& map = get<T>();
        map.erase(e.id);
    }
};

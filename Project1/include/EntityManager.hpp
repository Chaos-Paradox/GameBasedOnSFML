#pragma once
#include <unordered_set>
#include "Entity.hpp"
#include "ComponentManager.hpp"

class EntityManager {
public:
    EntityManager(ComponentManager& cm) : cm(cm) {}

    // 创建实体
    Entity create() {
        Entity e{ nextId++ };
        alive.insert(e.id);
        return e;
    }

    // 删除实体
    void destroy(Entity e) {
        // 移除实体
        if (alive.count(e.id)) {
            alive.erase(e.id);
            cm.destroyEntity(e);  // 委托给 ComponentManager 处理组件删除
        }
    }

    // 判断实体是否存在
    bool exists(Entity e) {
        return alive.count(e.id) > 0;
    }

private:
    int nextId = 0;  // 用于生成唯一的实体ID
    std::unordered_set<int> alive;  // 存储当前活跃的实体
    ComponentManager& cm;  // 引用 ComponentManager 来管理组件
};

#pragma once
#include <vector>
#include <queue>
#include "Entity.h"
#include "Component.h"

/**
 * @brief ECS 核心类
 * 
 * 负责实体的创建和销毁
 * 提供 Component 的添加/移除接口
 * 
 * ⚠️ Bug 1 修复：实现 Free List 实体回收机制
 */
class ECS {
public:
    Entity next = 1;
    std::vector<Entity> created_order;
    std::queue<Entity> freeList;  // ← 新增：实体 ID 回收队列

    Entity create() {
        Entity e;
        
        // ← 优先从回收队列复用 ID
        if (!freeList.empty()) {
            e = freeList.front();
            freeList.pop();
        } else {
            // 回收队列为空，分配新 ID
            e = next++;
        }
        
        created_order.push_back(e);
        std::cout << "[ECS] Created entity " << e << "\n";
        return e;
    }

    void destroy(Entity e) {
        // 1. 从活动实体列表中移除
        for (auto it = created_order.begin(); it != created_order.end(); ++it) {
            if (*it == e) {
                created_order.erase(it);
                break;
            }
        }
        
        // 2. ← 【关键修复】将 ID 加入回收队列，等待复用
        freeList.push(e);
    }
    
    // ← Bug 1 修复：添加 Component 接口
    template<typename T, typename Store>
    void addComponent(Entity e, Store& store) {
        // 简化实现：添加默认构造的 Component
        store.add(e, T{});
    }
    
    template<typename T, typename Store>
    void addComponent(Entity e, Store& store, const T& comp) {
        store.add(e, comp);
    }
    
    template<typename T, typename Store>
    void removeComponent(Entity e, Store& store) {
        store.remove(e);
    }

    const std::vector<Entity>& entities() const { return created_order; }
};

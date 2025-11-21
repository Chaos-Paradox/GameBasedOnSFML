#pragma once
#include <vector>
#include "Entity.h"
#include "Component.h"

class ECS {
public:
    Entity next = 1;
    std::vector<Entity> created_order;

    Entity create() {
        Entity e = next++;
        created_order.push_back(e);
        std::cout << "[ECS] Created entity " << e << "\n";
        return e;
    }

    const std::vector<Entity>& entities() const { return created_order; }
};

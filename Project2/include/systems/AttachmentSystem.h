#pragma once
#include "core/GameWorld.h"
#include <iostream>

/**
 * @brief 依附同步系统
 */
class AttachmentSystem {
public:
    void update(GameWorld& world, float dt)
    {
        (void)dt;

        auto attachedEntities = world.attachedComponents.entityList();

        for (Entity hitbox : attachedEntities) {
            if (!world.attachedComponents.has(hitbox)) continue;

            auto& attached = world.attachedComponents.get(hitbox);
            if (attached.owner == INVALID_ENTITY) continue;
            if (!world.transforms.has(attached.owner)) continue;

            const auto& ownerTransform = world.transforms.get(attached.owner);

            if (world.transforms.has(hitbox)) {
                auto& hitboxTransform = world.transforms.get(hitbox);
                hitboxTransform.position.x = ownerTransform.position.x + attached.offset.x;
                hitboxTransform.position.y = ownerTransform.position.y + attached.offset.y;
            }

            if (world.zTransforms.has(attached.owner) && world.zTransforms.has(hitbox)) {
                const auto& ownerZ = world.zTransforms.get(attached.owner);
                auto& hitboxZ = world.zTransforms.get(hitbox);
                hitboxZ.z = ownerZ.z;
            }
        }
    }
};

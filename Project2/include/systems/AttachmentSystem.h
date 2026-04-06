#pragma once
#include "../core/Component.h"
#include "../core/ECS.h"
#include "../components/AttachedComponent.h"
#include "../components/Transform.h"
#include "../components/ZTransformComponent.h"
#include <iostream>

/**
 * @brief 依附同步系统
 * 
 * ⚠️ 架构设计：
 * - 每帧同步依附实体（Hitbox、特效等）到主人实体
 * - 同步 XY 位置（owner.position + offset）
 * - 同步 Z 轴高度（直接拷贝 owner.z）
 * 
 * @see AttachedComponent - 依附组件
 */
class AttachmentSystem {
public:
    void update(
        ComponentStore<AttachedComponent>& attachedComponents,
        ComponentStore<TransformComponent>& transforms,
        ComponentStore<ZTransformComponent>& zTransforms,
        float dt)
    {
        (void)dt;
        
        auto attachedEntities = attachedComponents.entityList();
        
        for (Entity hitbox : attachedEntities) {
            if (!attachedComponents.has(hitbox)) continue;
            
            auto& attached = attachedComponents.get(hitbox);
            
            // 检查主人是否存在
            if (attached.owner == INVALID_ENTITY) continue;
            if (!transforms.has(attached.owner)) continue;
            
            const auto& ownerTransform = transforms.get(attached.owner);
            
            // ========== 强制同步 XY 位置 ==========
            if (transforms.has(hitbox)) {
                auto& hitboxTransform = transforms.get(hitbox);
                hitboxTransform.position.x = ownerTransform.position.x + attached.offset.x;
                hitboxTransform.position.y = ownerTransform.position.y + attached.offset.y;
            }
            
            // ========== 强制同步 Z 轴高度 ==========
            if (zTransforms.has(attached.owner) && zTransforms.has(hitbox)) {
                const auto& ownerZ = zTransforms.get(attached.owner);
                auto& hitboxZ = zTransforms.get(hitbox);
                hitboxZ.z = ownerZ.z;  // 直接拷贝 Z 高度
                // 注意：不拷贝 vz 和 gravity，Hitbox 不受重力影响
            }
        }
    }
};

#pragma once
#include "../math/Rect.h"
#include "../core/Entity.h"
#include <cstdint>

enum class ElementType : uint8_t {
    Physical,
    Fire,
    Toxic,
    Ice,
};

/**
 * @brief Hitbox（攻击盒）组件 - 纯数据（POD）
 */
struct HitboxComponent {
    Rect bounds;
    int damageMultiplier{100};
    ElementType element{};
    float knockbackForce{100.0f};
    Entity sourceEntity{INVALID_ENTITY};
    
    static constexpr int MAX_HIT_COUNT = 16;
    Entity hitHistory[MAX_HIT_COUNT]{};
    int hitCount{0};
    
    bool active{false};
};

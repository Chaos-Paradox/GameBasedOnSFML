# 迁移指南 - 从旧架构到新架构

> **版本:** 1.0  
> **最后更新:** 2026-03-29  
> **目标:** 将现有代码迁移到符合 `00_ARCHITECTURE.md` 和 `01_DATA_SCHEMA.md` 的新架构

---

## 概述

本次迁移的核心目标：

1. **Component 纯数据化** - 移除所有成员函数
2. **Tag 驱动通讯** - 跨系统通过 TagComponent 通讯
3. **命名统一** - 与 `01_DATA_SCHEMA.md` 一致

---

## 第一阶段：StateMachineComponent 重构

### 修改前

```cpp
// include/components/StateMachine.h
struct StateMachine {
    IState* currentState = nullptr;
    IState* previousState = nullptr;
    StateType stateType = StateType::None;
    
    void ChangeState(Entity entity, IState* newState) { ... }
    void Update(Entity entity, float dt) { ... }
};
```

### 修改后

```cpp
// include/components/StateMachine.h
#pragma once
#include "../states/IState.h"

/**
 * @brief 状态机组件（纯数据）
 * 
 * ⚠️ 注意：本组件只存储状态数据，不包含任何逻辑
 * 状态切换逻辑在 StateMachineSystem 中执行
 */
struct StateMachineComponent {
    CharacterState currentState{CharacterState::Idle};
    CharacterState previousState{CharacterState::Idle};
    float stateTimer{0.0f};
};
```

### System 层迁移

```cpp
// include/systems/StateMachineSystem.h
class StateMachineSystem {
public:
    void update(
        ComponentStore<StateMachineComponent>& states,
        const ComponentStore<InputCommandComponent>& inputs,
        const std::vector<Entity>& entities,
        float dt)
    {
        for (Entity e : entities) {
            auto& state = states.get(e);
            
            // ✅ 直接读写数据，不调用方法
            state.stateTimer += dt;
            
            // 检测 Tag（伤害）
            if (damageTags.has(e)) {
                state.previousState = state.currentState;
                state.currentState = CharacterState::Hurt;
                damageTags.remove(e); // 消费后销毁
            }
            
            // 检测输入
            if (inputs.has(e)) {
                auto& input = inputs.get(e);
                if (input.moveDirection != sf::Vector2f(0, 0)) {
                    state.currentState = CharacterState::Move;
                }
            }
        }
    }
};
```

---

## 第二阶段：创建 TagComponent

### 新增文件

```cpp
// include/components/DamageTag.h
#pragma once
#include "../utils/CollisionTypes.h"

/**
 * @brief 伤害交接 Tag
 * 
 * 生命周期：1 帧
 * CollisionSystem 挂载 → StateMachineSystem 消费 → 销毁
 */
struct DamageTagComponent {
    int rawDamage{0};
    ElementType element{ElementType::Physical};
    sf::Vector2f knockbackDirection{0.0f, 0.0f};
    EntityId attackerId{INVALID_ENTITY_ID};
    float timestamp{0.0f};
};
```

```cpp
// include/components/Tags.h
#pragma once
#include "DamageTag.h"

// 其他 Tags...

/**
 * @brief 进化指令 Tag
 */
struct EvolveCommandTag {
    uint32_t skillId{0};
    int level{1};
};

/**
 * @brief 交互指令 Tag
 */
struct InteractCommandTag {
    EntityId targetEntity{INVALID_ENTITY_ID};
};

/**
 * @brief 合成指令 Tag
 */
struct CraftCommandTag {
    uint32_t recipeId{0};
    int craftCount{1};
};
```

---

## 第三阶段：Hitbox/Hurtbox 重构

### 修改前

```cpp
struct Hitbox {
    Rect rect;
    Rect getWorldRect(float x, float y) { ... }  // ❌ 成员函数
    void activate() { ... }  // ❌ 成员函数
};
```

### 修改后

```cpp
// include/components/Hitbox.h
#pragma once
#include <SFML/System/Vector2.hpp>
#include <SFML/Graphics/Rect.hpp>

struct HitboxComponent {
    sf::FloatRect bounds;
    int damageMultiplier{100};
    ElementType element{ElementType::Physical};
    float knockbackForce{100.0f};
    EntityId sourceEntity{INVALID_ENTITY_ID};
    std::set<EntityId> hitHistory;
    bool active{false};
};

// 工具方法移到 Helper 类
class HitboxHelper {
public:
    static sf::FloatRect getWorldRect(
        const HitboxComponent& hitbox,
        const sf::Vector2f& position)
    {
        return {
            position.x + hitbox.bounds.left,
            position.y + hitbox.bounds.top,
            hitbox.bounds.width,
            hitbox.bounds.height
        };
    }
    
    static void activate(HitboxComponent& hitbox) {
        hitbox.active = true;
    }
    
    static void deactivate(HitboxComponent& hitbox) {
        hitbox.active = false;
    }
};
```

---

## 第四阶段：CombatSystem Tag 驱动

### 修改前

```cpp
// CollisionSystem::update
std::vector<CollisionEvent> update(...) {
    std::vector<CollisionEvent> events;
    
    if (collision) {
        events.push_back({attacker, victim, damage});
    }
    
    return events;
}

// CombatSystem::handleEvents
void handleEvents(const std::vector<CollisionEvent>& events) {
    for (const auto& event : events) {
        stats.get(event.victim).hp -= event.damage;  // ❌ 直接修改
    }
}
```

### 修改后

```cpp
// CollisionSystem::update
void update(
    const ComponentStore<HitboxComponent>& hitboxes,
    const ComponentStore<HurtboxComponent>& hurtboxes,
    const ComponentStore<TransformComponent>& transforms,
    ComponentStore<DamageTagComponent>& damageTags,
    const std::vector<Entity>& entities)
{
    for (Entity attacker : entities) {
        if (!hitboxes.has(attacker) || !hitboxes.get(attacker).active) continue;
        
        for (Entity victim : entities) {
            if (attacker == victim) continue;
            if (!hurtboxes.has(victim)) continue;
            
            if (checkCollision(attacker, victim)) {
                // ✅ 挂载 Tag，不直接处理
                damageTags.add(victim, {
                    .rawDamage = hitboxes.get(attacker).damageMultiplier,
                    .element = hitboxes.get(attacker).element,
                    .attackerId = attacker,
                    .timestamp = currentTime
                });
            }
        }
    }
}
```

```cpp
// StateMachineSystem::update
void update(...) {
    for (Entity e : entities) {
        auto& state = states.get(e);
        
        // ✅ 检测 Tag
        if (damageTags.has(e)) {
            state.previousState = state.currentState;
            state.currentState = CharacterState::Hurt;
            state.stateTimer = 0.0f;
            
            // 消费后销毁
            damageTags.remove(e);
        }
    }
}
```

---

## 第五阶段：命名统一

### Component 重命名映射

| 旧名称 | 新名称 | 文件 |
|--------|--------|------|
| `StateMachine` | `StateMachineComponent` | `components/StateMachine.h` |
| `Hitbox` | `HitboxComponent` | `components/Hitbox.h` |
| `Hurtbox` | `HurtboxComponent` | `components/Hurtbox.h` |
| `Position` | `TransformComponent` | `components/Transform.h` |
| `Stats` | `CharacterComponent` | `components/Character.h` |
| `InputCommand` | `InputCommandComponent` | `components/InputCommand.h` |

### 使用示例

```cpp
// 旧代码
ComponentStore<StateMachine> stateMachines;
ComponentStore<Hitbox> hitboxes;
ComponentStore<Position> positions;

// 新代码
ComponentStore<StateMachineComponent> states;
ComponentStore<HitboxComponent> hitboxes;
ComponentStore<TransformComponent> transforms;
```

---

## 检查清单

### 代码检查

- [ ] 所有 Component 无成员函数
- [ ] 所有 Component 是 POD（Plain Old Data）
- [ ] 跨系统通讯使用 TagComponent
- [ ] 无直接系统调用（如 `system.changeState()`）
- [ ] 命名与 `01_DATA_SCHEMA.md` 一致

### 文档检查

- [ ] `features/F001-StateMachine.md` 更新
- [ ] `features/F201-Collision.md` 更新
- [ ] `CODE_REVIEW.md` 记录
- [ ] `CHANGELOG.md` 更新

### 测试检查

- [ ] 所有现有测试通过
- [ ] 新增 Tag 驱动测试
- [ ] 性能测试通过

---

## 常见问题

### Q: 为什么 Component 不能有成员函数？

**A:** ECS 的核心优势是数据连续性和多线程友好。如果 Component 包含逻辑：
- 无法批量处理（数据不连续）
- 多线程访问冲突
- 难以网络同步（需要序列化逻辑）

### Q: TagComponent 的生命周期是多久？

**A:** 通常只有 1 帧：
```
Frame N:   SystemA 挂载 Tag
Frame N:   SystemB 检测并消费 Tag
Frame N+1: Tag 已销毁（或系统清理过期 Tag）
```

### Q: 工具方法应该放在哪里？

**A:** 创建 Helper 类：
```cpp
class HitboxHelper {
    static sf::FloatRect getWorldRect(...);
    static void activate(...);
};
```

---

## 相关文档

- [`00_ARCHITECTURE.md`](00_ARCHITECTURE.md) - 架构规范
- [`01_DATA_SCHEMA.md`](01_DATA_SCHEMA.md) - 数据字典
- [`CODE_REVIEW.md`](CODE_REVIEW.md) - 代码审查报告
- [`features/F001-StateMachine.md`](features/F001-StateMachine.md) - 状态机系统

---

**维护者:** Project2 Team  
**最后更新:** 2026-03-29

# F002 - Tag 系统

> **版本:** 1.0  
> **状态:** ✅ 设计完成  
> **优先级:** 🔴 最高

---

## 概述

TagComponent 是跨系统通讯的**唯一合法方式**。通过挂载临时 Tag，实现系统间的解耦和数据交接。

**核心特性:**
- 生命周期短暂（通常 1 帧）
- 纯数据，无逻辑
- 生产者挂载，消费者销毁
- 便于网络同步

---

## 设计目标

- ✅ 系统间零耦合
- ✅ 数据交接可追溯
- ✅ 便于调试和日志
- ✅ 网络同步友好

---

## 系统设计

### Tag 生命周期

```
Frame N:
┌──────────────┐
│ System A     │
│ 生产者       │
│              │
│ 挂载 Tag     │
└──────┬───────┘
       │
       ▼
┌──────────────┐
│ ECS World    │
│ ComponentStore│
│              │
│ Tag 存在     │
└──────┬───────┘
       │
       ▼
┌──────────────┐
│ System B     │
│ 消费者       │
│              │
│ 检测 Tag     │
│ 处理逻辑     │
│ 销毁 Tag     │
└──────────────┘

Frame N+1:
Tag 已销毁（或自动清理）
```

### Tag 与 Component 的区别

| 特性 | Component | Tag |
|------|-----------|-----|
| **生命周期** | 长期（实体存在期间） | 短暂（1 帧或几帧） |
| **用途** | 存储实体状态 | 跨系统通讯 |
| **数量** | 每个实体一份 | 按需挂载 |
| **清理** | 实体销毁时 | 消费后立即销毁 |

---

## 数据结构

### DamageTagComponent

```cpp
struct DamageTagComponent {
    int rawDamage{0};
    ElementType element{ElementType::Physical};
    sf::Vector2f knockbackDirection{0.0f, 0.0f};
    EntityId attackerId{INVALID_ENTITY_ID};
    float timestamp{0.0f}; // 挂载时间
};
```

### EvolveCommandTag

```cpp
struct EvolveCommandTag {
    uint32_t skillId{0};
    int level{1};
};
```

### InteractCommandTag

```cpp
struct InteractCommandTag {
    EntityId targetEntity{INVALID_ENTITY_ID};
};
```

### CraftCommandTag

```cpp
struct CraftCommandTag {
    uint32_t recipeId{0};
    int craftCount{1};
};
```

### SpawnCommandTag

```cpp
struct SpawnCommandTag {
    uint32_t enemyTypeId{0};
    sf::Vector2f spawnPosition{0.0f, 0.0f};
    int count{1};
};
```

---

## API 接口

### ComponentStore 操作

```cpp
// 挂载 Tag
damageTags.add(victim, {
    .rawDamage = 10,
    .element = ElementType::Physical,
    .attackerId = attacker,
    .timestamp = currentTime
});

// 检测 Tag
if (damageTags.has(entity)) {
    auto& tag = damageTags.get(entity);
    // 处理...
}

// 销毁 Tag（消费后）
damageTags.remove(entity);

// 清理过期 Tag（帧末）
void cleanupExpiredTags(float currentTime, float maxAge = 1.0f) {
    for (auto& [entity, tag] : damageTags) {
        if (currentTime - tag.timestamp > maxAge) {
            damageTags.remove(entity);
        }
    }
}
```

---

## 使用示例

### 战斗伤害交接

```cpp
// CollisionSystem（生产者）
void CollisionSystem::update(
    ...,
    ComponentStore<DamageTagComponent>& damageTags)
{
    for (Entity attacker : entities) {
        for (Entity victim : entities) {
            if (checkCollision(attacker, victim)) {
                // ✅ 挂载 Tag
                damageTags.add(victim, {
                    .rawDamage = hitboxes.get(attacker).damage,
                    .attackerId = attacker,
                    .timestamp = currentTime
                });
            }
        }
    }
}

// StateMachineSystem（消费者）
void StateMachineSystem::update(
    ComponentStore<StateMachineComponent>& states,
    ComponentStore<DamageTagComponent>& damageTags)
{
    for (Entity e : entities) {
        auto& state = states.get(e);
        
        // ✅ 检测 Tag
        if (damageTags.has(e)) {
            state.previousState = state.currentState;
            state.currentState = CharacterState::Hurt;
            state.stateTimer = 0.0f;
            
            // ✅ 消费后销毁
            damageTags.remove(e);
        }
    }
}
```

### UI 进化指令

```cpp
// UI 层（生产者）
void onEvolveButtonClicked(uint32_t skillId) {
    evolveCommandTags.add(playerEntity, {
        .skillId = skillId,
        .level = 1
    });
}

// ProgressionSystem（消费者）
void ProgressionSystem::update(
    ComponentStore<EvolutionComponent>& evolutions,
    ComponentStore<EvolveCommandTag>& tags)
{
    for (Entity e : entities) {
        if (tags.has(e)) {
            auto& tag = tags.get(e);
            auto& evolution = evolutions.get(e);
            
            if (evolution.evolutionPoints >= getSkillCost(tag.skillId)) {
                evolution.evolutionPoints -= getSkillCost(tag.skillId);
                evolution.activeSkills.push_back(tag.skillId);
            }
            
            tags.remove(e); // 消费后销毁
        }
    }
}
```

---

## 依赖关系

- **Project1 ECS:** ComponentStore

---

## 测试用例（规划中）

```
[ ] Tag 挂载测试
[ ] Tag 检测测试
[ ] Tag 销毁测试
[ ] Tag 生命周期测试
[ ] 多 Tag 并发测试
[ ] 过期 Tag 清理测试
```

---

## 性能评估

### 时间复杂度

- **挂载:** O(1)
- **检测:** O(1)
- **销毁:** O(1)
- **清理:** O(n)，n=Tag 数量

### 性能预算

```
Tag 系统预算：< 0.1ms/帧
- 每帧平均 10-20 个 Tag
- 单次操作：< 0.005ms
```

---

## 已知问题

### 待设计项

- [ ] Tag 优先级（多个 Tag 同时存在）
- [ ] Tag 堆叠（同一类型多个 Tag）
- [ ] Tag 持久化（超过 1 帧的生命周期）

---

## 更新历史

### [2026-03-29] - 初始设计

**状态:** 设计完成

**备注:** Tag 系统是 ECS 架构的核心，用于跨系统通讯

---

## 相关文档

- [00_ARCHITECTURE.md](../00_ARCHITECTURE.md) - 架构规范
- [01_DATA_SCHEMA.md](../01_DATA_SCHEMA.md) - 数据字典
- [F201-Collision.md](F201-Collision.md) - 碰撞系统（Tag 生产者）
- [F001-StateMachine.md](F001-StateMachine.md) - 状态机系统（Tag 消费者）
- [MIGRATION_GUIDE.md](../MIGRATION_GUIDE.md) - 迁移指南

---

**维护者:** Project2 Team  
**最后更新:** 2026-03-29

# 架构修复报告 - 第一阶段

> **修复日期:** 2026-03-29  
> **修复范围:** Component 纯数据化 + Tag 系统创建  
> **状态:** ✅ 已完成

---

## 修复总览

| 修复项 | 状态 | 文件数 |
|--------|------|--------|
| StateMachineComponent 重构 | ✅ 完成 | 1 |
| TagComponent 创建 | ✅ 完成 | 2 |
| Hitbox/Hurtbox 重构 | ✅ 完成 | 2 |
| StateMachineSystem 更新 | ✅ 完成 | 1 |
| CollisionSystem 更新 | ✅ 完成 | 1 |
| CharacterComponent 创建 | ✅ 完成 | 1 |

---

## 详细修复

### 1. StateMachineComponent 重构 ✅

**文件:** `include/components/StateMachine.h`

**修改前:**
```cpp
struct StateMachine {
    IState* currentState;
    void ChangeState(Entity, IState*);  // ❌ 成员函数
    void Update(Entity, float dt);      // ❌ 成员函数
};
```

**修改后:**
```cpp
struct StateMachineComponent {
    CharacterState currentState{CharacterState::Idle};
    CharacterState previousState{CharacterState::Idle};
    float stateTimer{0.0f};
    // ✅ 无成员函数，纯数据
};
```

---

### 2. TagComponent 创建 ✅

**新增文件:**
- `include/components/DamageTag.h`
- `include/components/Tags.h`

**DamageTagComponent:**
```cpp
struct DamageTagComponent {
    int rawDamage{0};
    ElementType element{ElementType::Physical};
    sf::Vector2f knockbackDirection;
    EntityId attackerId{INVALID_ENTITY_ID};
    float timestamp{0.0f};
};
```

**其他 Tags:**
- `EvolveCommandTag` - UI → ProgressionSystem
- `InteractCommandTag` - Player → InteractionSystem
- `CraftCommandTag` - Player → CraftingSystem
- `SpawnCommandTag` - Director → World

---

### 3. Hitbox/Hurtbox 重构 ✅

**文件:** `include/components/Hitbox.h`, `include/components/Hurtbox.h`

**修改前:**
```cpp
struct Hitbox {
    Rect rect;
    Rect getWorldRect();    // ❌ 成员函数
    void activate();        // ❌ 成员函数
};
```

**修改后:**
```cpp
struct HitboxComponent {
    sf::FloatRect bounds;
    int damageMultiplier{100};
    bool active{false};
    // ✅ 无成员函数，纯数据
};

class HitboxHelper {
    static sf::FloatRect getWorldRect(...);  // ✅ 工具方法在 Helper 类
    static void activate(...);
};
```

---

### 4. StateMachineSystem 更新 ✅

**文件:** `include/systems/StateMachineSystem.h`

**关键改动:**
```cpp
// ✅ Tag 驱动
void update(
    ComponentStore<StateMachineComponent>& states,
    const ComponentStore<InputCommand>& inputs,
    const ComponentStore<DamageTagComponent>& damageTags,  // ← 新增
    ...)
{
    for (Entity e : entities) {
        auto& state = states.get(e);
        
        // ✅ 检测 Tag
        if (damageTags.has(e)) {
            state.previousState = state.currentState;
            state.currentState = CharacterState::Hurt;
            // Tag 由生产者销毁，这里只读
        }
    }
}
```

---

### 5. CollisionSystem 更新 ✅

**文件:** `include/systems/CollisionSystem.h`

**关键改动:**
```cpp
// ✅ 挂载 Tag，不直接处理伤害
void update(
    ...,
    ComponentStore<DamageTagComponent>& damageTags)  // ← 新增
{
    if (checkCollision(attacker, victim)) {
        // ✅ 挂载 Tag
        damageTags.add(victim, {
            .rawDamage = hitbox.damageMultiplier,
            .attackerId = attacker,
            .timestamp = currentTime
        });
    }
}
```

---

### 6. CharacterComponent 创建 ✅

**文件:** `include/components/Character.h`

**说明:** 重命名 `Stats.h` → `Character.h`，符合数据字典规范

```cpp
struct CharacterComponent {
    std::string name;
    int level{1};
    int maxHP{100};
    int currentHP{100};
    // ... 符合 01_DATA_SCHEMA.md
};
```

---

## 架构合规性检查

### ✅ 符合原则

| 原则 | 状态 |
|------|------|
| Component 是 POD | ✅ 所有 Component 无成员函数 |
| 跨系统用 Tag 通讯 | ✅ DamageTag 等已创建 |
| 命名统一 | ✅ Component 后缀统一 |
| 渲染解耦 | ✅ 准备中（RenderEventComponent） |

### ⚠️ 待完成

| 项目 | 优先级 |
|------|--------|
| 迁移现有代码使用新 Component | 🔴 高 |
| 创建 RuntimeStatsComponent | 🟡 中 |
| 创建 EvolutionComponent | 🟡 中 |
| 渲染层解耦实现 | 🟢 低 |

---

## 使用示例

### 战斗流程（Tag 驱动）

```cpp
// 1. CollisionSystem 检测碰撞
collisionSystem.update(hitboxes, hurtboxes, positions, damageTags, entities, time);

// 2. StateMachineSystem 检测 Tag
stateMachineSystem.update(states, inputs, damageTags, entities, dt);

// 3. Tag 自动销毁（由 CollisionSystem 或清理系统处理）
```

### 添加新 Tag

```cpp
// 1. 定义 Tag
struct MyNewTag {
    int data;
};

// 2. 生产者挂载
myTags.add(entity, {42});

// 3. 消费者检测并销毁
if (myTags.has(entity)) {
    // 处理...
    myTags.remove(entity);
}
```

---

## 下一步行动

### 第二阶段（下周）

1. **创建剩余 Component**
   - `RuntimeStatsComponent`
   - `EvolutionComponent`
   - `InventoryComponent`

2. **更新测试代码**
   - 使用新 Component 名称
   - 添加 Tag 驱动测试

3. **文档更新**
   - 更新 `features/F001-StateMachine.md`
   - 更新 `features/F201-Collision.md`

---

## 相关文档

- [`00_ARCHITECTURE.md`](00_ARCHITECTURE.md) - 架构规范
- [`01_DATA_SCHEMA.md`](01_DATA_SCHEMA.md) - 数据字典
- [`CODE_REVIEW.md`](CODE_REVIEW.md) - 代码审查报告
- [`MIGRATION_GUIDE.md`](MIGRATION_GUIDE.md) - 迁移指南
- [`features/F002-TagSystem.md`](features/F002-TagSystem.md) - Tag 系统

---

**修复者:** AI Assistant  
**审核状态:** ✅ 第一阶段完成  
**下次审查:** 第二阶段修复后

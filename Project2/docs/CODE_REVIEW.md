# 代码审查报告 - 架构合规性检查

> **审查日期:** 2026-03-29  
> **审查标准:** `docs/00_ARCHITECTURE.md` + `docs/01_DATA_SCHEMA.md`  
> **审查范围:** Project2/include 所有头文件

---

## 📊 审查总览

| 检查项 | 通过 | 失败 | 警告 |
|--------|------|------|------|
| Component 纯数据 | ❌ 0 | ✅ 4 | ⚠️ 3 |
| 无成员函数 | ❌ 2 | ✅ 5 | - |
| Tag 驱动通讯 | ❌ 未实现 | ✅ 规划中 | - |
| 渲染解耦 | ⚠️ 部分 | - | ⚠️ 1 |
| 命名规范 | ✅ 7 | ❌ 0 | - |

---

## ❌ 严重违规（必须立即修复）

### 1. StateMachineComponent 包含成员函数

**文件:** `include/components/StateMachine.h`

**问题:**
```cpp
struct StateMachine {
    // ❌ 违规：Component 包含业务逻辑
    void ChangeState(Entity entity, IState* newState) { ... }
    void Update(Entity entity, float dt) { ... }
    void RevertToPreviousState(Entity entity) { ... }
    bool IsInState(StateType type) const { ... }
};
```

**违反原则:**
- `01_DATA_SCHEMA.md`: "所有的 Component 必须是 POD，严禁包含任何业务逻辑函数"
- `00_ARCHITECTURE.md`: "Component 只能是纯数据结构"

**修复方案:**
```cpp
// ✅ 正确做法：纯数据
struct StateMachineComponent {
    CharacterState currentState{CharacterState::Idle};
    CharacterState previousState{CharacterState::Idle};
    float stateTimer{0.0f};
};

// 逻辑移到 StateMachineSystem
class StateMachineSystem {
    void changeState(Entity entity, CharacterState newState);
    void update(Entity entity, float dt);
};
```

**优先级:** 🔴 最高（架构基石）

---

### 2. Hitbox/Hurtbox 包含成员函数

**文件:** `include/components/Hitbox.h`, `include/components/Hurtbox.h`

**问题:**
```cpp
struct Hitbox {
    // ❌ 违规：Component 包含工具方法
    Rect getWorldRect(float entityX, float entityY) const { ... }
    void activate() { ... }
    void deactivate() { ... }
    void set(float x, float y, ...) { ... }
};
```

**违反原则:**
- `01_DATA_SCHEMA.md`: "Component 只能是纯数据结构"

**修复方案:**
```cpp
// ✅ 正确做法：纯数据
struct HitboxComponent {
    sf::FloatRect bounds;
    int damageMultiplier{100};
    ElementType element{ElementType::Physical};
    float knockbackForce{100.0f};
    EntityId sourceEntity{INVALID_ENTITY_ID};
    std::set<EntityId> hitHistory;
    bool active{false};
};

// 工具方法移到辅助类
class HitboxHelper {
    static sf::FloatRect getWorldRect(const HitboxComponent&, const TransformComponent&);
    static void activate(HitboxComponent&);
};
```

**优先级:** 🔴 高

---

### 3. 缺少 TagComponent（跨系统通讯）

**问题:** 当前代码直接调用系统方法，未使用 Tag 驱动。

**当前做法（❌ 错误）:**
```cpp
// CollisionSystem 直接生成事件
auto events = collision.update(...);
for (const auto& event : events) {
    stats.get(event.victim).hp -= event.damage;
}
```

**正确做法（✅ Tag 驱动）:**
```cpp
// CollisionSystem 挂载 Tag
damageTags.add(victim, {
    .rawDamage = hitbox.damage,
    .attackerId = attacker
});

// StateMachineSystem 检测 Tag
if (damageTags.has(entity)) {
    stateMachine.currentState = CharacterState::Hurt;
    damageTags.remove(entity); // 消费后销毁
}
```

**缺失的 Tags:**
- `DamageTagComponent` - 伤害交接
- `EvolveCommandTag` - 进化指令
- `InteractCommandTag` - 交互指令
- `CraftCommandTag` - 合成指令
- `SpawnCommandTag` - 生成指令

**优先级:** 🔴 高（影响网络同步）

---

## ⚠️ 警告（建议修复）

### 1. StatsComponent 命名不一致

**文件:** `include/components/Stats.h`

**问题:**
```cpp
struct Stats {  // ❌ 命名不规范
    float hp = 100.0f;
    float attackDamage = 10.0f;
};
```

**规范:** `01_DATA_SCHEMA.md` 使用 `CharacterComponent`

**修复:** 重命名为 `CharacterComponent` 或 `StatsComponent`

**优先级:** 🟡 中

---

### 2. PositionComponent 字段命名

**文件:** `include/components/Position.h`

**问题:**
```cpp
struct Position {
    float x, y;  // ⚠️ 应该使用 sf::Vector2f
};
```

**规范:** `01_DATA_SCHEMA.md` 使用 `TransformComponent` 和 `sf::Vector2f position`

**优先级:** 🟡 中

---

### 3. StateMachineSystem 直接调用状态方法

**文件:** `include/systems/StateMachineSystem.h`

**问题:**
```cpp
sm.ChangeState(e, &IdleState::Instance());  // ❌ 直接调用
sm.Update(e, dt);  // ❌ 直接调用
```

**正确做法:**
```cpp
// System 只读写 Component 数据
auto& state = stateMachines.get(e);
state.currentState = CharacterState::Idle;  // ✅ 只写数据
state.stateTimer = 0.0f;
```

**优先级:** 🟡 中（重构工作量大）

---

## ✅ 合规项（保持）

### 1. IState 接口设计

**文件:** `include/states/IState.h`

**评价:** ✅ 符合状态模式，接口清晰

**备注:** 状态类的实现应该在 `features/F001-StateMachine.md` 中详细说明

---

### 2. CollisionSystem 系统职责

**文件:** `include/systems/CollisionSystem.h`

**评价:** ✅ 系统职责清晰，只处理碰撞检测

**改进:** 使用 TagComponent 代替直接返回事件列表

---

### 3. 状态枚举定义

**文件:** `include/states/IState.h`

**评价:** ✅ `StateType` 枚举符合规范

---

## 📋 修复优先级清单

### 第一阶段（架构基石，本周）

1. **重构 StateMachineComponent** - 移除所有成员函数
2. **创建 TagComponent 头文件** - 实现跨系统通讯
3. **更新 StateMachineSystem** - 使用 Tag 驱动

### 第二阶段（数据纯净，下周）

4. **重构 Hitbox/Hurtbox** - 移除工具方法
5. **统一 Component 命名** - 与 `01_DATA_SCHEMA.md` 一致
6. **创建 Helper 类** - 迁移工具方法

### 第三阶段（系统优化，下月）

7. **实现 DamageTag 交接** - CollisionSystem → StateMachineSystem
8. **实现 EvolveCommandTag** - UI → ProgressionSystem
9. **渲染解耦** - 使用 RenderEventComponent

---

## 📝 需要补充的文档

### 功能文档（features/）

| 文档 | 状态 | 优先级 |
|------|------|--------|
| `F002-TagSystem.md` | ❌ 缺失 | 🔴 高 |
| `F003-StateMachineSystem.md` | ❌ 缺失 | 🔴 高 |
| `F202-CombatSystem.md` | ❌ 缺失 | 🟡 中 |
| `F302-ModifierSystem.md` | ❌ 缺失 | 🟡 中 |

### 代码规范文档

| 文档 | 状态 | 优先级 |
|------|------|--------|
| `CODE_REVIEW.md` | ✅ 本文档 | - |
| `MIGRATION_GUIDE.md` | ❌ 缺失 | 🔴 高 |
| `EXAMPLES.md` | ❌ 缺失 | 🟡 中 |

---

## 🎯 迁移指南（快速参考）

### StateMachineComponent 迁移

```cpp
// 旧代码（❌）
struct StateMachine {
    IState* currentState;
    void ChangeState(Entity, IState*);
};

// 新代码（✅）
struct StateMachineComponent {
    CharacterState currentState{CharacterState::Idle};
    CharacterState previousState{CharacterState::Idle};
    float stateTimer{0.0f};
};
```

### Tag 驱动示例

```cpp
// CollisionSystem 中
void update(...) {
    if (collision) {
        // ❌ 旧做法：直接处理
        stats.get(victim).hp -= damage;
        
        // ✅ 新做法：挂载 Tag
        damageTags.add(victim, {damage, attacker});
    }
}

// StateMachineSystem 中
void update(...) {
    if (damageTags.has(entity)) {
        auto& tag = damageTags.get(entity);
        state.currentState = CharacterState::Hurt;
        damageTags.remove(entity); // 消费后销毁
    }
}
```

---

**审查者:** AI Assistant  
**审核状态:** 待修复  
**下次审查:** 修复后重新审查

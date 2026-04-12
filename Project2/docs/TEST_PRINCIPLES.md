# Project2 测试原则

> **版本:** 1.0  
> **最后更新:** 2026-03-29  
> **文件:** `test/test_principles.h`

---

## 概述

本测试框架采用四层架构，确保代码质量和 ECS 架构的纯洁性。

```
┌─────────────────────────────────────────────────────────┐
│  L3: Pipeline & Tag Integration Tests                   │
│  (跨系统链路集成测试)                                    │
├─────────────────────────────────────────────────────────┤
│  L2: Isolated Logic Tests                               │
│  (单一 System 隔离测试)                                   │
├─────────────────────────────────────────────────────────┤
│  L1: Factory Assembly Tests                             │
│  (实体装配测试)                                         │
├─────────────────────────────────────────────────────────┤
│  L0: Paradigm & Memory Tests                            │
│  (底层规范防线)                                         │
└─────────────────────────────────────────────────────────┘
```

---

## L0: 底层规范防线 (Paradigm & Memory Tests)

### 目的

这层测试不测业务逻辑，只测"规矩"。防止未来的你在疲惫时破坏了 ECS 的纯洁性。

### 测试内容

#### L0-Test-1: Component 的 POD 验证

**目的:** 确保没有任何人往 Component 里偷偷塞虚函数或复杂的非连续内存对象。

**为什么重要:**
- 保证缓存命中率（连续内存）
- 网络序列化成本低（memcpy 即可）
- 多线程安全（无虚表指针竞争）

**断言样例:**
```cpp
static_assert(std::is_standard_layout_v<HitboxComponent>,
    "HitboxComponent must be standard layout (POD)");
static_assert(std::is_trivial_v<HitboxComponent>,
    "HitboxComponent must be trivial (no constructors)");
```

**验证的 Component:**
- TransformComponent
- CharacterComponent
- StateMachineComponent
- HitboxComponent
- HurtboxComponent
- DamageTagComponent
- InventoryComponent
- RuntimeStatsComponent
- EvolutionComponent

#### L0-Test-2: 无虚函数验证

**目的:** 确保 Component 没有虚函数表。

**断言样例:**
```cpp
static_assert(!std::is_polymorphic_v<TransformComponent>,
    "TransformComponent must not have virtual functions");
```

#### L0-Test-3: 无依赖初始化

**目的:** 验证 System 的实例化不需要任何外部依赖。

**验证内容:**
- StateMachineSystem 可以无依赖实例化
- CollisionSystem 可以无依赖实例化
- 不依赖 SFML Window、渲染上下文等

---

## L1: 实体装配测试 (Factory Assembly Tests)

### 目的

专门测试"实体制造工厂"，确保下放到业务层的"零件"没有错漏。

### 测试内容

#### L1-Test-1: 完整性装配验证

**情境:** 调用 `PlayerFactory::createPlayer(...)`

**断言:**
- ✅ TransformComponent 存在
- ✅ CharacterComponent 存在
- ✅ StateMachineComponent 存在
- ✅ InventoryComponent 存在
- ❌ AIComponent 不存在（玩家不需要）

**宏定义:**
```cpp
L1_FACTORY_ASSEMBLY_TEST(ecs, transforms, characters, states, inventories, player)
```

#### L1-Test-2: 初始值默认验证

**情境:** 检查刚刚生成的玩家实体

**断言:**
- `Character.currentHP == Character.maxHP` (满血)
- `StateMachine.currentState == Idle` (待机状态)
- `Transform.position == expectedPos` (初始位置正确)

**宏定义:**
```cpp
L1_INITIAL_VALUES_TEST(characters, states, transforms, player, expectedPos)
```

---

## L2: 单一 System 隔离测试 (Isolated Logic Tests)

### 目的

ECS 的魅力在于你可以把某一个 System 单独拎出来跑。为每一个核心 System 编写无外部干扰的测试。

### 测试内容

#### L2-Test-1: MovementSystem（位移演算）

**情境:**
- 创建一个空实体
- 只挂载 Transform(pos: 0,0) 和速度 (100, 0)
- StateMachine 处于 Move 状态

**行为:** 手动调用 `MovementSystem.update(0.1f)`

**断言:**
- `Transform.pos == (10, 0)` (速度 100 × 时间 0.1)

**宏定义:**
```cpp
L2_MOVEMENT_SYSTEM_TEST(ecs, transforms, characters, states, moveSystem)
```

#### L2-Test-2: ModifierSystem（RPG 属性结算）

**情境:**
- 基础攻击力：10
- 修饰器 1: `+50%`
- 修饰器 2: `+5`

**行为:** 运行 `ModifierSystem.update()`

**断言:**
- `RuntimeStats.finalAttack == 20` (10 + 10×0.5 + 5 = 20)

**宏定义:**
```cpp
L2_MODIFIER_SYSTEM_TEST(ecs, characters, modifiers, runtimeStats, modifierSystem)
```

---

## L3: 跨系统链路集成测试 (Pipeline & Tag Integration Tests)

### 目的

这是最重要的一层，专门测试系统之间的"交接仪式"是否顺畅，特别是 Tag 驱动机制。

### 测试内容

#### L3-Test-1: 完整的"受击 -> 扣血 -> 状态切换"闭环

**情境:**
1. 初始化满血 player (HP: 100, 状态：Idle)
2. 挂载 DamageTagComponent{rawDamage: 20}（模拟 CombatSystem）

**行为:** 调用 `StateMachineSystem.update()`

**断言:**
1. `Character.currentHP == 80` (扣血)
2. `StateMachine.currentState == Hurt` (状态切换)
3. **极其关键:** `DamageTagComponent` 必须被彻底销毁（防止下一帧重复扣血）

**宏定义:**
```cpp
L3_COMBAT_TAG 闭环_TEST(ecs, characters, states, damageTags, stateSystem)
```

#### L3-Test-2: 无敌帧的拦截测试

**情境:**
- 挂载了 DamageTag
- `Character.isInvincible == true`

**行为:** 运行系统

**断言:**
- HP 不变
- 状态不切换
- **Tag 仍被销毁**（防止内存泄漏）

**宏定义:**
```cpp
L3_INVINCIBLE_INTERCEPT_TEST(ecs, characters, states, damageTags, stateSystem)
```

---

## 使用指南

### 运行测试

```bash
cd <项目根目录>
cmake --build build --target test_architecture
./build/bin/test_architecture
```

### 添加新测试

1. 在 `test_principles.h` 中定义宏
2. 在 `test_architecture.cpp` 中调用宏
3. 确保测试独立、可重复

### 测试命名规范

```
L{层级}-Test-{编号}: {测试名称}

示例:
L0-Test-1: Component POD 验证
L1-Test-2: 初始值默认验证
L2-Test-1: MovementSystem 位移演算
L3-Test-1: 战斗 Tag 闭环
```

---

## 测试金字塔

```
          ╱╲
         ╱  ╲
        ╱ L3 ╲         集成测试 (10%)
       ╱──────╲
      ╱   L2   ╲       单元测试 (20%)
     ╱──────────╲
    ╱     L1     ╲     装配测试 (30%)
   ╱──────────────╲
  ╱       L0       ╲   规范测试 (40%)
 ╱──────────────────╲
```

**原则:**
- L0 测试最多（编译期静态断言）
- L3 测试最少（集成测试运行慢）
- 每层都有明确的职责和边界

---

## 最佳实践

### ✅ 正确做法

```cpp
// L0: 使用静态断言（编译期检查）
static_assert(std::is_standard_layout_v<T>, "Must be POD");

// L1: 测试 Factory 的完整性
assert(transforms.has(player) && "Must have Transform");

// L2: 隔离测试单个 System
MovementSystem moveSystem;
moveSystem.update(transforms, states, {entity}, dt);

// L3: 验证 Tag 生命周期
damageTags.add(entity, tag);
system.update(...);
assert(!damageTags.has(entity) && "Tag must be destroyed");
```

### ❌ 错误做法

```cpp
// ❌ 在 L0 测试业务逻辑
assert(character.currentHP == 100); // 这是 L1/L2 的内容

// ❌ 在 L2 测试多个 System
moveSystem.update(...);
combatSystem.update(...); // 这是 L3 的内容

// ❌ 忘记清理 Tag
damageTags.add(entity, tag);
system.update(...);
// ❌ 没有验证 Tag 是否销毁
```

---

## 相关文档

- [`test_principles.h`](test/test_principles.h) - 测试宏定义
- [`test_architecture.cpp`](test/test_architecture.cpp) - 测试实现
- [`CODE_REVIEW.md`](docs/CODE_REVIEW.md) - 代码审查规范
- [`00_ARCHITECTURE.md`](docs/00_ARCHITECTURE.md) - 架构规范

---

**维护者:** Project2 Team  
**最后更新:** 2026-03-29

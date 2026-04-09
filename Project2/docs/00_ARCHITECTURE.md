# 00_ARCHITECTURE.md - Project2 架构地图

> **多人共斗游戏核心架构，与渲染解耦**  
> **版本:** 3.1（单轨覆盖指令槽） | **最后更新:** 2026-04-06

---

## ⚠️ 编码规范与约束（防 AI 幻觉护城河）

**违反以下规则的代码将被直接拒绝。**

### 内存分配

```
❌ 严禁在 System 的 update() 循环中使用 new、malloc 或任何动态内存分配。
✅ 所有实体必须通过 World::createEntity() 生成。
✅ 所有组件必须通过 ComponentStore::add() 添加。
✅ 状态对象必须使用单例模式（State::Instance()）。
```

### 跨模块交互（渲染解耦）

```
❌ 游戏逻辑层（Project2）绝对不允许包含任何 SFML 或渲染相关的头文件。
❌ 禁止：#include <SFML/Graphics.hpp>
❌ 禁止：#include <SFML/System/Vector2.hpp>
✅ 只能使用自定义数学类型：Vec2, Rect
✅ 只能通过往 ECS 里压入 EventComponent 来通知外部渲染层。
✅ 只能通过修改 Component 数据来间接影响渲染。
```

### 系统间通讯（纯正 ECS 原则）

```
❌ System 之间严禁持有对方的指针。
❌ 严禁：CombatSystem 直接调用 StateMachineSystem::changeState()
❌ 严禁：AISystem 直接访问 CollisionSystem 的内部数据
✅ System 之间必须通过查阅 Component 数据进行状态同步。
✅ 使用 TagComponent 进行跨系统信号传递（见下方数据交接协议）。
```

### 数据交接协议（Data-Driven）

```
当 CombatSystem 需要触发角色受伤时：

❌ 错误做法（直接调用）:
    stateMachine.changeState(StateType::Hurt);  // 严禁！

✅ 正确做法（Tag 驱动）:
    // 1. CombatSystem 挂载 Tag
    damageTags.add(victim, {damage: 10, source: attacker});
    
    // 2. StateMachineSystem 在下一帧检测 Tag
    if (damageTags.has(entity)) {
        changeState(entity, StateType::Hurt);
        damageTags.remove(entity);  // 销毁 Tag
    }
```

### 组件使用规范

```
❌ 严禁在 Component 中包含任何逻辑（虚函数、方法实现）。
✅ Component 只能是纯数据结构（struct，只有字段）。
❌ 严禁：struct Transform { void update() {...} };
✅ 正确：struct Transform { Vec2 position; Vec2 velocity; };
```

### 实体生命周期

```
❌ 严禁在 System::update() 中直接销毁实体。
✅ 必须使用延迟销毁队列。
✅ 在 update() 中标记，在帧末统一处理。
```

---

## 🗺️ 领域文档地图 (Domain Map)

**当需要修改具体逻辑时，请严格参考以下子文档：**

### 核心战斗（已完成 ✅）

| 功能领域 | 文档位置 | 内容 |
|----------|----------|------|
| **状态机与角色行为** | [`docs/features/F001-StateMachine.md`](features/F001-StateMachine.md) | Idle/Move/Attack/Hurt/Dead 状态详解 |
| **碰撞与伤害结算** | [`docs/features/F201-Collision.md`](features/F201-Collision.md) | Hitbox/Hurtbox、碰撞检测、伤害计算 |
| **攻击系统** | [`docs/features/F103-Attack.md`](features/F103-Attack.md) | 攻击触发、Hitbox 生成、连招 |
| **受伤状态** | [`docs/features/F104-Hurt.md`](features/F104-Hurt.md) | 无敌时间、状态恢复 |
| **死亡状态** | [`docs/features/F105-Death.md`](features/F105-Death.md) | 终态处理、游戏结束 |

### 战利品与拾取（已完成 ✅）

| 功能领域 | 文档位置 | 内容 |
|----------|----------|------|
| **战利品掉落** | [`docs/features/F202-LootDrop.md`](features/F202-LootDrop.md) | 掉落表、LootSpawnSystem、爆米花抛射 |
| **拾取系统** | [`docs/features/F203-Pickup.md`](features/F203-Pickup.md) | PickupBox、拾取判定、延迟销毁 |
| **磁吸系统** | [`docs/features/F204-Magnet.md`](features/F204-Magnet.md) | 玩家吸收半径、免疫计时器、摩擦力 |

### RPG 与进化（规划中 📋）

| 功能领域 | 文档位置 | 内容 |
|----------|----------|------|
| **进化与成长** | `docs/features/F301-Evolution.md` | 进化点数、技能树、被动突变 |
| **属性修饰器** | `docs/features/F302-Modifier.md` | 动态增益计算、最终面板 |
| **等级与经验** | `docs/features/F303-Progression.md` | 经验获取、升级、发放点数 |

### AI 与游戏流程（规划中 📋）

| 功能领域 | 文档位置 | 内容 |
|----------|----------|------|
| **AI 系统** | `docs/features/F202-AI.md` | 巡逻、警戒、战斗 AI |
| **游戏流程** | `docs/features/F203-GameFlow.md` | 菜单、暂停、游戏结束 |

---

## 🏛️ 架构分层 (Layered Architecture)

```
┌─────────────────────────────────────────────────────────┐
│                   渲染层 (外部)                          │
│  (SFML / OpenGL / Vulkan - 可插拔)                       │
└─────────────────────────────────────────────────────────┘
                          ↓ 读取 RenderEventComponent
┌─────────────────────────────────────────────────────────┐
│               Project2 游戏逻辑层                        │
│  (纯 C++, 无渲染依赖，可独立编译)                         │
├─────────────────────────────────────────────────────────┤
│  System 层 (无状态，纯逻辑)                              │
│  ┌──────────────┐ ┌──────────────┐ ┌──────────────┐   │
│  │StateMachine  │ │  Collision   │ │   Combat     │   │
│  │   System     │ │   System     │ │   System     │   │
│  └──────────────┘ └──────────────┘ └──────────────┘   │
├─────────────────────────────────────────────────────────┤
│  Component 层 (纯数据，POD)                              │
│  ┌──────────────┐ ┌──────────────┐ ┌──────────────┐   │
│  │  Transform   │ │   Hitbox     │ │  DamageTag   │   │
│  │  StateMachine│ │   Hurtbox    │ │  Position    │   │
│  └──────────────┘ └──────────────┘ └──────────────┘   │
├─────────────────────────────────────────────────────────┤
│  数学库 (自定义，无 SFML)                                │
│  ┌──────────────┐ ┌──────────────┐                     │
│  │    Vec2      │ │     Rect     │                     │
│  └──────────────┘ └──────────────┘                     │
└─────────────────────────────────────────────────────────┘
```

---

## 📊 数据流管线 (Data Flow Pipeline)

### 单帧执行顺序

```
Frame Start
    │
    ├─→ InputSystem (读取玩家输入)
    │       └─→ 写入 InputCommand 组件
    │
    ├─→ StateMachineSystem (状态决策)
    │       ├─→ 读取 InputCommand
    │       ├─→ 读取/写入 StateMachineComponent
    │       └─→ 切换状态（Enter/Update/Exit）
    │
    ├─→ MovementSystem (移动计算)
    │       └─→ 写入 TransformComponent.velocity
    │
    ├─→ CollisionSystem (碰撞检测)
    │       ├─→ 读取 Hitbox/Hurtbox/Position
    │       └─→ 写入 DamageTag (如果碰撞)
    │
    ├─→ CombatSystem (伤害结算)
    │       ├─→ 读取 DamageTag
    │       ├─→ 写入 CharacterComponent.currentHP
    │       └─→ 销毁 DamageTag
    │
    ├─→ DeathSystem (死亡处理)
    │       ├─→ 检查 CharacterComponent.currentHP ≤ 0
    │       └─→ 挂载 DeathTag
    │
    ├─→ LootSpawnSystem ⭐ (掉落生成)
    │       ├─→ 读取 DeathTag + LootDropComponent
    │       ├─→ 生成掉落物 (Transform + ItemData + PickupBox)
    │       └─→ 设置爆米花抛射初速度
    │
    ├─→ MagnetSystem ⭐ (磁吸吸附)
    │       ├─→ 遍历玩家 (MagnetComponent)
    │       ├─→ 检查掉落物免疫计时器
    │       ├─→ 如果在吸收半径内：设置飞向玩家速度
    │       └─→ 如果脱离半径：摩擦力减速
    │
    ├─→ PickupSystem ⭐ (拾取判定)
    │       ├─→ 检测玩家与掉落物接触
    │       ├─→ 增加玩家进化点数
    │       └─→ 标记掉落物待销毁
    │
    ├─→ CleanupSystem ⭐ (清理销毁)
    │       ├─→ 减少 LifetimeComponent.timeLeft
    │       └─→ 销毁到期实体
    │
    ├─→ RenderSystem (外部，读取 RenderEventComponent)
    │
    └─→ 清理延迟销毁队列
    
Frame End
```

---

## 🔧 数学类型（无 SFML 依赖）

### Vec2 - 二维向量

```cpp
#include "math/Vec2.h"

struct Vec2 {
    float x, y;
    
    // 运算符
    constexpr Vec2 operator+(const Vec2& other) const;
    constexpr Vec2 operator-(const Vec2& other) const;
    constexpr Vec2 operator*(float scalar) const;
    
    // 方法
    constexpr float length() const;
    constexpr Vec2 normalized() const;
    constexpr float dot(const Vec2& other) const;
};
```

**替代：** `sf::Vector2f`

---

### Rect - 矩形 AABB

```cpp
#include "math/Rect.h"

struct Rect {
    float x, y;          // 左上角
    float width, height;
    
    // 边界
    constexpr float left() const;
    constexpr float right() const;
    constexpr float top() const;
    constexpr float bottom() const;
    
    // 碰撞检测
    constexpr bool overlaps(const Rect& other) const;
    constexpr bool contains(float px, float py) const;
};
```

**替代：** `sf::FloatRect`

---

### MathUtils - 工具函数

```cpp
#include "math/MathUtils.h"

namespace Math {
    constexpr float distance(const Vec2& a, const Vec2& b);
    constexpr float angle(const Vec2& from, const Vec2& to);
    constexpr bool inSector(const Vec2& center, const Vec2& target, 
                           float maxDistance, float maxAngleRadians);
    constexpr float lerp(float a, float b, float t);
    constexpr float clamp(float value, float min, float max);
}
```

---

## 📋 Component 清单

完整 Component 列表请查看：[`COMPONENT_LIST.md`](COMPONENT_LIST.md)

### 核心 Component

| Component | 字段 | 用途 |
|-----------|------|------|
| `TransformComponent` | `Vec2 position, scale, velocity`<br>`float rotation` | 位置、旋转、速度 |
| `StateMachineComponent` | `CharacterState currentState`<br>`CharacterState previousState`<br>`float stateTimer` | 状态机数据 |
| `CharacterComponent` | `int level, maxHP, currentHP`<br>`int baseAttack, baseDefense`<br>`float baseMoveSpeed` | 角色属性 |
| `InputCommand` | `Vec2 moveDir`<br>`ActionIntent pendingIntent`<br>`float intentTimer` | **单轨指令槽**（v3.1） |

---

## 🎮 输入系统架构（v3.1 单轨覆盖指令槽）

### 设计动机

**问题：** 传统多 Timer 独立缓存在长硬直场景下会失效

```
场景：玩家被击中，进入 1.0 秒 Hurt 硬直
- 第 0.5 秒：玩家按下冲刺键
- 第 0.7 秒：0.2 秒保质期耗尽，指令过期
- 第 1.0 秒：硬直结束，角色呆立原地（玩家大骂！）
```

### 解决方案：单轨覆盖 + 时间静止魔法

**核心原则：**

1. **单一意图槽** - `ActionIntent pendingIntent` 代替多个独立 Timer
2. **Last-In-Wins** - 新指令无条件覆盖旧指令，重置保质期
3. **时间静止** - Hurt/Dead/Dash 期间 `intentTimer` 暂停倒计时

**架构蓝图：**

```cpp
enum class ActionIntent {
    None,
    Attack,
    Dash
};

struct InputCommand {
    Vec2 moveDir{0.0f, 0.0f};
    ActionIntent pendingIntent{ActionIntent::None};  // ← 单一意图槽
    float intentTimer{0.0f};                         // ← 统一保质期
};
```

**输入录入（VisualSandbox.cpp）：**

```cpp
// Last-In-Wins：新指令覆盖旧指令
if (currentJPressed && !lastJPressed) {
    inputs.get(player).pendingIntent = ActionIntent::Attack;
    inputs.get(player).intentTimer = 0.2f;
}
if (currentSpacePressed && !lastSpacePressed) {
    inputs.get(player).pendingIntent = ActionIntent::Dash;  // ← 覆盖攻击
    inputs.get(player).intentTimer = 0.2f;                  // ← 重置保质期
}
```

**时间静止魔法（StateMachineSystem）：**

```cpp
if (input.intentTimer > 0.0f) {
    // 只在可行动状态下倒计时
    if (state.currentState != CharacterState::Hurt &&
        state.currentState != CharacterState::Dead &&
        state.currentState != CharacterState::Dash) {
        input.intentTimer -= dt;
    }
}
```

**效果：**

- ✅ 硬直期间按下的指令永远保鲜，直到硬直结束
- ✅ 同时按 J+Space 以最后按下的为准，无逻辑死锁
- ✅ 精准消费：只在成功切入状态时清零意图

### 战斗 Component

| Component | 字段 | 用途 |
|-----------|------|------|
| `HitboxComponent` | `Rect bounds`<br>`int damageMultiplier`<br>`ElementType element`<br>`EntityId hitHistory[MAX_HIT_COUNT]`<br>`bool active` | 攻击判定框 |
| `HurtboxComponent` | `Rect bounds`<br>`Faction faction`<br>`int layer`<br>`float invincibleTime` | 受击判定框 |
| `DamageTag` | `float damage`<br>`Vec2 knockbackDirection`<br>`EntityId source`<br>`bool applied` | 伤害交接 Tag |
| `AttackStateComponent` | `float hitTimer`<br>`bool hitActivated` | 攻击状态锁 |
| `DeathTag` | `float timestamp` | 死亡标记（1 帧） |

### 战利品 Component（新增 ✅）

| Component | 字段 | 用途 |
|-----------|------|------|
| `LootDropComponent` | `LootEntry lootTable[8]`<br>`int lootCount`<br>`bool hasDropped` | 怪物掉落表 |
| `ItemDataComponent` | `unsigned int itemId`<br>`int amount`<br>`float magnetImmunityTimer` | 掉落物数据 |
| `PickupBoxComponent` | `float width, height` | 拾取碰撞框 |
| `MagnetComponent` | `float magnetRadius`<br>`float magnetSpeed` | 玩家吸收半径 |
| `LifetimeComponent` | `float timeLeft`<br>`bool autoDestroy` | 临时实体生命周期 |

### 冲刺与输入 Component（新增 ✅）

| Component | 字段 | 用途 |
|-----------|------|------|
| `DashComponent` | `float dashSpeed`<br>`float dashDuration`<br>`float iframeDuration`<br>`float cooldown`<br>`float dashTimer`<br>`float iframeTimer`<br>`Vec2 dashDir`<br>`bool isInvincible` | 冲刺能力数据 |
| `InputCommand` | `Vec2 moveDir`<br>`float attackBufferTimer` | 输入命令（限时缓存） |

---

## 🎯 设计原则

### 1. Data-Oriented Design

- Component 是纯数据（POD）
- System 是无状态函数
- 数据连续存储，缓存友好

### 2. 渲染隔离

- 游戏逻辑不依赖任何渲染库
- 通过 RenderEventComponent 通知渲染层
- 可轻松切换渲染后端

### 3. Tag 驱动通信

- System 间不直接调用
- 通过 TagComponent 传递事件
- 生命周期：1 帧（自动清理）

### 4. 单例模式

- State 对象使用单例
- 零内存开销
- 线程安全（C++11 static）

---

## 📝 更新历史

| 版本 | 日期 | 改动 | 负责人 |
|------|------|------|--------|
| **3.0** | **2026-04-06** | **完整战斗系统：冲刺、攻击取消、限时输入缓存、帧暂停** | **AI Assistant** |
| 2.2 | 2026-04-03 | 添加战利品与拾取系统（Loot/Magnet/Pickup） | AI Assistant |
| 2.1 | 2026-04-03 | 修复幽灵惯性 Bug，添加爆米花抛射效果 | AI Assistant |
| 2.0 | 2026-03-29 | 移除所有 SFML 依赖，使用自定义数学类型 | AI Assistant |
| 1.0 | 2026-03-29 | 初始版本 | Project2 Team |

---

**维护者:** Project2 Team  
**最后更新:** 2026-04-06  
**下次审查:** 添加新 System 或 Component 时

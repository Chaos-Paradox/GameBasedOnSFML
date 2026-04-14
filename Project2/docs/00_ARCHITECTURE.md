# 00_ARCHITECTURE.md - Project2 模块架构

> **多人共斗动作游戏核心架构，与渲染解耦**
> **版本:** 4.0（JSON 驱动实体工厂 + 2.5D Z 轴物理 + 电荷炸弹系统） | **最后更新:** 2026-04-15

---

## 一、项目概述

这是一个 C++20 + SFML 3.x 的双人共斗动作游戏，采用 **ECS（Entity-Component-System）** 架构。代码库分为两个项目：

- **Project1** — 核心 ECS 框架（EntityManager、ComponentManager、EventBus）
- **Project2** — 游戏逻辑（纯 C++，渲染无关），包含战斗、战利品、AI 导演、炸弹物理等系统

游戏逻辑刻意与渲染层解耦 —— Project2 使用自定义 `Vec2`/`Rect` 数学类型，通过 `RenderEventComponent` 和 `GameJuice` 与渲染层通信。

---

## ⚠️ 编码规范与约束（防 AI 幻觉护城河）

**违反以下规则的代码将被拒绝。**

### 内存分配

```
❌ 严禁在 System 的 update() 循环中使用 new、malloc 或任何动态内存分配。
✅ 所有实体必须通过 ECS.create() 生成。
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
当 CollisionSystem 需要触发角色受伤时：

❌ 错误做法（直接调用）:
    stateMachine.changeState(StateType::Hurt);  // 严禁！

✅ 正确做法（Tag 驱动）:
    // 1. CollisionSystem 创建 DamageEventComponent 实体
    damageEvents.add(newEntity, {damage: 10, source: attacker});

    // 2. DamageSystem 在下一帧检测 DamageEventComponent
    if (damageEvents.has(entity)) {
        applyDamage(entity);
        changeState(entity, StateType::KnockedAirborne);
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
✅ 必须使用延迟销毁队列（DeathTag + CleanupSystem）。
✅ 在 update() 中标记，在帧末统一处理。
```

---

## 二、模块目录结构

```
Project2/
├── CMakeLists.txt              # 核心库构建配置
├── include/
│   ├── components/             # [组件层] 35 个 POD 数据结构
│   ├── systems/                # [系统层] 21 个无状态处理系统
│   ├── core/                   # [ECS 核心] 实体管理、组件存储、世界上下文
│   ├── states/                 # [状态层] IState 接口 + 6 个具体状态
│   ├── factories/              # [工厂层] 实体创建工厂
│   ├── math/                   # [数学层] Vec2, Rect, MathUtils
│   └── utils/                  # [工具层] 碰撞类型、日志、数学工具
├── src/
│   └── tools.cpp               # 核心库源文件（空，逻辑在沙盒中）
├── tests/
│   ├── sandbox/                # 可视化测试沙盒（SFML + ImGui）
│   │   └── VisualSandbox.cpp   # 主入口 + 帧循环
│   ├── unit/                   # 单元测试（GTest）
│   └── standalone/             # 独立测试
├── test/
│   └── loot_pipeline_test.cpp  # GTest 战利品管线测试
├── data/                       # 游戏数据（JSON 预制体）
├── docs/
│   ├── 00_ARCHITECTURE.md      # 本文档
│   ├── 01_DATA_SCHEMA.md       # 数据 schema 文档
│   ├── COMPONENT_LIST.md       # 组件完整清单
│   └── features/               # 功能领域文档
├── external/
│   ├── imgui/                  # Dear ImGui
│   └── imgui-sfml/             # ImGui-SFML 绑定
└── material/
    └── fonts/                  # 字体资源
```

---

## 三、模块职责

### 3.1 Core 模块（ECS 核心）

| 文件 | 职责 |
|------|------|
| `ECS.h` | 实体创建/销毁，Free List ID 回收机制 |
| `Entity.h` | `Entity` 类型别名（`uint32_t`），`INVALID_ENTITY` 哨兵值 |
| `Component.h` | `ComponentStore<T>` 泛型组件容器（`unordered_map<Entity, T>`） |
| `GameWorld.h` | 中央枢纽：持有所有 ComponentStore、ECS、GameJuice、InputManager、玩家引用 |
| `EntityFactory.h` | JSON 驱动实体工厂，从预制体文件生成实体 |
| `InputManager.h` | 双玩家独立输入管理器，JSON 持久化键位配置 |
| `GameJuice.h` | 全局游戏感觉状态（hit-stop、camera shake、time scale） |
| `GameConfig.h` | 游戏配置（空占位文件） |

### 3.2 Math 模块（自定义数学库）

| 文件 | 职责 | 替代 SFML |
|------|------|----------|
| `Vec2.h` | 2D 向量：运算符、length/normalized/dot、常量 | `sf::Vector2f` |
| `Rect.h` | AABB 矩形：overlap/contains/translate/scale | `sf::FloatRect` |
| `MathUtils.h` | 工具函数：distance/angle/inSector/lerp/clamp | — |

### 3.3 Components 模块（35 个 POD 结构）

按功能域分为 9 类，详见 [`COMPONENT_LIST.md`](COMPONENT_LIST.md)。

### 3.4 Systems 模块（21 个无状态系统）

按功能域分为 7 类，详见 [`MODULES.md`](MODULES.md)。

### 3.5 States 模块（状态机）

| 文件 | 状态 | 职责 |
|------|------|------|
| `IState.h` | — | 状态接口：Enter/Update/Exit/GetName/GetType |
| `IdleState.h` | Idle | 静止待命 |
| `MoveState.h` | Move | 移动中 |
| `AttackState.h` | Attack | 攻击动作 |
| `HurtState.h` | Hurt | 受击硬直 |
| `DeadState.h` | Dead | 死亡终态 |

### 3.6 Factories 模块

| 文件 | 职责 |
|------|------|
| `EntityFactory.h` | JSON 驱动工厂：spawnPlayer、spawnDummy、spawnThrowableBomb、spawnExplosion |
| `PlayerFactory.h` | 遗留硬编码工厂（已被 EntityFactory 取代） |

### 3.7 Utils 模块

| 文件 | 职责 |
|------|------|
| `CollisionTypes.h` | 碰撞层位掩码、阵营枚举、碰撞事件配置、碰撞矩阵 |
| `Logging.h` | 组件字符串化、帧调试输出 |
| `Math.h` | 遗留扇形检测工具（已被 MathUtils 取代） |
| `Rect.h` | 遗留碰撞工具（已被 math/Rect.h 取代） |

---

## 四、架构分层

```
┌─────────────────────────────────────────────────────────────────┐
│                        渲染层 (外部)                             │
│  SFML / ImGui / OpenGL — VisualSandbox.cpp 中的 RenderSystem     │
└─────────────────────────────────────────────────────────────────┘
                              ↓ 读取 GameJuice + 所有 Component
┌─────────────────────────────────────────────────────────────────┐
│                    Project2 游戏逻辑层                            │
│  (纯 C++, 无渲染依赖，可独立编译)                                 │
├─────────────────────────────────────────────────────────────────┤
│  System 层 (21 个无状态系统)                                     │
│  ┌──────────┐ ┌──────────┐ ┌──────────┐ ┌──────────┐          │
│  │ State    │ │ Combat   │ │ Physics  │ │ Loot     │          │
│  │ Machine  │ │ Pipeline │ │ Collision│ │ Pickup   │          │
│  └──────────┘ └──────────┘ └──────────┘ └──────────┘          │
├─────────────────────────────────────────────────────────────────┤
│  Component 层 (35 个 POD 结构)                                   │
│  Transform, Character, Hitbox, Hurtbox, DamageTag, Dash, ...   │
├─────────────────────────────────────────────────────────────────┤
│  数学库 (自定义，无 SFML)                                        │
│  Vec2, Rect, MathUtils                                          │
└─────────────────────────────────────────────────────────────────┘
```

---

## 五、单帧执行顺序（VisualSandbox.cpp 实际管线）

```
Frame Start
    │
    ├─→ [Phase 1: 状态决策]
    │   ├─→ StateMachineSystem     ← 读取 InputCommand, 决策状态切换
    │   └─→ DashSystem             ← 处理冲刺计时、电荷回复、无敌帧
    │
    ├─→ [Phase 2: 移动计算]
    │   ├─→ LocomotionSystem       ← 根据输入方向 + baseMoveSpeed 计算 velocity
    │   ├─→ MagnetSystem           ← 计算玩家对掉落物的磁吸速度
    │   └─→ BombSystem             ← 炸弹倒计时、踢飞碰撞、爆炸 AOE
    │
    ├─→ [Phase 3: 物理与碰撞]
    │   ├─→ MovementSystem         ← 速度积分 → 位置更新，摩擦力，1800px/s 限速
    │   ├─→ PhysicalCollisionSystem ← 实体间物理碰撞（弹性碰撞 + CCD）
    │   └─→ AttachmentSystem       ← 同步附属实体（如 Hitbox）到宿主位置
    │
    ├─→ [Phase 4: 战斗管线]
    │   ├─→ AttackSystem           ← 扇形扫描 → 创建 DamageEventComponent
    │   ├─→ CollisionSystem        ← Hitbox-vs-Hurtbox 2.5D 圆柱碰撞 → DamageEvent
    │   └─→ DamageSystem           ← 处理伤害事件：扣血、击飞、GameJuice、死亡标记
    │
    ├─→ [Phase 5: 战后处理]
    │   ├─→ LootSpawnSystem        ← 读取 DeathTag + LootDrop → 生成掉落物
    │   ├─→ DeathSystem            ← 读取 DeathTag → 设为 Dead 状态（玩家免疫）
    │   └─→ PickupSystem           ← 玩家-掉落物接触判定 → 增加进化点
    │
    ├─→ [Phase 6: 清理]
    │   └─→ CleanupSystem          ← 销毁 DeathTag / Lifetime 到期实体
    │
    ├─→ [Phase 7: 渲染]
    │   ├─→ RenderSystem           ← 2.5D Y 排序渲染（需要 SFML）
    │   ├─→ DamageTextRenderSystem ← 浮动伤害数字渲染（需要 SFML）
    │   └─→ DebugSystem            ← 控制台调试输出（可选）
    │
    └─→ Frame End
```

---

## 六、系统依赖关系图

```
                    ┌─────────────┐
                    │ InputManager│
                    │ (SFML 事件) │
                    └──────┬──────┘
                           │ 写入
                           ▼
                    ┌──────────────┐
                    │ InputCommand │◄── StateMachineSystem ("时间静止魔法")
                    └──────┬──────┘
                           │ 读取
                           ▼
┌─────────────────┐  ┌──────────────┐  ┌──────────────┐
│ LocomotionSystem│  │  DashSystem  │  │  BombSystem  │
│ (移动速度计算)   │  │ (冲刺处理)    │  │ (炸弹逻辑)    │
└────────┬────────┘  └──────┬───────┘  └──────┬───────┘
         │                  │                 │
         ▼                  ▼                 ▼
┌─────────────────────────────────────────────────────┐
│               TransformComponent.velocity            │
└────────────────────────┬────────────────────────────┘
                         │
                         ▼
                 ┌───────────────┐
                 │MovementSystem │ ← 位置积分 + 摩擦力
                 └───────┬───────┘
                         │
                         ▼
           ┌─────────────────────────────┐
           │  PhysicalCollisionSystem    │ ← 实体排斥 + CCD
           └──────────────┬──────────────┘
                          │
                          ▼
           ┌─────────────────────────────┐
           │   AttachmentSystem          │ ← 附属实体同步
           └─────────────────────────────┘
                          │
                          ▼
     ┌──────────┐    ┌──────────┐
     │AttackSys │    │Collision │ ← 2.5D 圆柱碰撞
     │ (扇形)   │    │  System  │
     └────┬─────┘    └────┬─────┘
          │               │
          ▼               ▼
     ┌─────────────────────────┐
     │  DamageEventComponent   │ ← 伤害事件实体
     └──────────┬──────────────┘
                │
                ▼
        ┌──────────────┐
        │ DamageSystem │ ← 扣血 + 击飞 + GameJuice
        └──────┬───────┘
               │
        ┌──────┴───────┐
        ▼              ▼
   ┌──────────┐  ┌──────────┐
   │DeathTag  │  │DamageText│
   └────┬─────┘  └──────────┘
        │
   ┌────┴─────────────┬──────────────┐
   ▼                  ▼              ▼
LootSpawn         DeathSys       PickupSys
(掉落生成)        (死亡状态)      (拾取判定)
   │                  │              │
   └──────────────────┴──────────────┘
                      │
                      ▼
               CleanupSystem (延迟销毁)
```

---

## 七、关键设计模式

### 7.1 GameWorld 注册表模式

`GameWorld` 持有所有 `ComponentStore` 容器，消除 System 间大量参数传递：

```cpp
GameWorld world;
movementSystem.update(world, dt);
// 内部通过 world.transforms.get(e)、world.ecs.create() 等访问
```

### 7.2 JSON 驱动实体工厂

`EntityFactory` 从 `data/` 目录的 JSON 预制体文件加载实体配置，支持热更新：

```cpp
EntityFactory factory;
factory.spawnPlayer(world, "data/prefabs/player.json");
factory.spawnThrowableBomb(world, "data/prefabs/bomb.json");
```

### 7.3 双玩家输入管理器

`InputManager` 支持 P1/P2 独立键位配置，JSON 持久化到磁盘：

- P1：WASD + J/K/Space/G
- P2：方向键 + 数字键盘

### 7.4 GameJuice 全局感觉系统

```cpp
struct GameJuice {
    float timeScale;        // hit-stop / slow-mo
    float hitStopTimer;     // 命中停顿计时
    float shakeTimer;       // 屏幕震动计时
    float shakeIntensity;   // 震动强度
};
```

DamageSystem 写入 → RenderSystem 消费。

### 7.5 2.5D Z 轴物理

`ZTransformComponent` 提供独立的 Z 轴（高度）计算，用于：

- 炸弹弹跳物理（重力 + 垂直速度）
- 击飞空中状态（KnockedAirborne）
- 碰撞系统 Z 轴过滤（空中实体不参与地面碰撞）

### 7.6 单轨覆盖指令槽（v3.1）

`InputCommand` 使用单一 `ActionIntent pendingIntent` 槽 + 统一 `intentTimer`：

- Last-In-Wins：新指令无条件覆盖旧指令，重置保质期
- 时间静止：Hurt/Dead/Dash/Recovery 期间 timer 暂停倒计时
- 精准消费：只在成功切入状态时清零意图

---

## 八、核心 Component 快速参考

### 空间类

| Component | 关键字段 | 用途 |
|-----------|---------|------|
| `TransformComponent` | position, velocity, facingX/Y | 主空间变换 |
| `ZTransformComponent` | z, vz, gravity, height | Z 轴高度（跳跃/炸弹弹跳） |
| `Position` | x, y | 简化位置（遗留） |
| `LifetimeComponent` | timeLeft, autoDestroy | 临时实体自动销毁 |

### 角色/状态类

| Component | 关键字段 | 用途 |
|-----------|---------|------|
| `CharacterComponent` | currentHP, maxHP, baseAttack, baseMoveSpeed | 角色基础属性 |
| `StateMachineComponent` | currentState, previousState, stateTimer | 状态机数据 |
| `InputCommand` | moveDir, pendingIntent, intentTimer | 单轨输入指令 |
| `DashComponent` | dashSpeed, maxCharges, iframeTimer | 冲刺能力 |

### 战斗类

| Component | 关键字段 | 用途 |
|-----------|---------|------|
| `HitboxComponent` | radius, offset, damageMultiplier, hitTargets | 攻击判定圆柱 |
| `HurtboxComponent` | radius, height, faction, invincibleTime | 受击判定圆柱 |
| `AttackStateComponent` | hitTimer, baseDamage, attackRange, hasFiredDamage | 攻击状态 |
| `DamageEventComponent` | actualDamage, hitDirection, knockbackXY/Z | 伤害事件载荷 |
| `DamageTag` | damage | 伤害交接 Tag（遗留） |
| `DeathTag` | (标记) | 延迟销毁标记 |

### 战利品类

| Component | 关键字段 | 用途 |
|-----------|---------|------|
| `LootDropComponent` | lootTable[8], lootCount, hasDropped | 怪物掉落表 |
| `ItemDataComponent` | itemId, amount, magnetImmunityTimer | 掉落物数据 |
| `PickupBoxComponent` | width, height | 拾取碰撞框 |
| `MagnetComponent` | magnetRadius, magnetSpeed | 玩家磁吸半径 |

### 物理类

| Component | 关键字段 | 用途 |
|-----------|---------|------|
| `ColliderComponent` | radius, isStatic | 物理碰撞圆柱 |
| `MomentumComponent` | mass, velocity, prevPosX/Y, useCCD | 动量 + CCD |
| `BombComponent` | fuseTimer, isKicked, lastPosX/Y | 炸弹实体 |

---

## 📝 更新历史

| 版本 | 日期 | 改动 | 负责人 |
|------|------|------|--------|
| **4.0** | **2026-04-15** | **模块架构完整分析：35 Components + 21 Systems + 9 模块分类** | **AI Assistant** |
| 3.1 | 2026-04-06 | 完整战斗系统：冲刺、攻击取消、限时输入缓存、帧暂停 | AI Assistant |
| 3.0 | 2026-04-03 | 添加战利品与拾取系统（Loot/Magnet/Pickup） | AI Assistant |
| 2.0 | 2026-03-29 | 移除所有 SFML 依赖，使用自定义数学类型 | AI Assistant |
| 1.0 | 2026-03-29 | 初始版本 | Project2 Team |

---

**维护者:** Project2 Team
**最后更新:** 2026-04-15
**下次审查:** 添加新 System 或 Component 时

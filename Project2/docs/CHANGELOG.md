# Project2 改动记录

> 记录所有重要的架构改动、功能新增和修复。  
> 格式：`[YYYY-MM-DD] - 改动类型 - 说明`

---

## [2026-03-29] - 架构修复 - 移除所有 SFML 依赖 ✅

**类型：** 架构重构  
**优先级：** 🔴 阻塞性修复  
**影响范围：** 所有 Component 和 System

### 改动说明

**问题：** 7 个 Component 文件包含 SFML 头文件，违反 prompt.md 禁令 4

**解决方案：**
1. 创建自定义数学类型库（`include/math/`）
2. 替换所有 SFML 类型为自定义类型
3. 修复 HitboxComponent 的 POD 合规性

### 新增文件

**数学库（3 个）：**
- `include/math/Vec2.h` - 二维向量（替代 `sf::Vector2f`）
- `include/math/Rect.h` - 矩形 AABB（替代 `sf::FloatRect`）
- `include/math/MathUtils.h` - 数学工具函数

### 修改文件

| 文件 | 改动 |
|------|------|
| `Transform.h` | `sf::Vector2f` → `Vec2` |
| `Hitbox.h` | `sf::FloatRect` → `Rect`，移除 `std::set` |
| `Hurtbox.h` | `sf::FloatRect` → `Rect` |
| `DamageTag.h` | `sf::Vector2f` → `Vec2` |
| `RenderEvent.h` | `sf::Vector2f` → `Vec2` |
| `DirectorState.h` | 移除未使用的 SFML 依赖 |
| `BiomeZone.h` | `sf::FloatRect` → `Rect` |
| `PlayerFactory.h` | `sf::Vector2f` → `Vec2` |
| `CollisionSystem.h` | 使用新数学类型 |

### 验证结果

```bash
# SFML 依赖检查
grep -r "#include <SFML" include/ src/
# 结果：✅ 未发现

# POD 合规检查
grep -r "private:\|virtual" include/components/
# 结果：✅ 所有 Component 都是纯 struct

# 编译验证
cmake --build build
# 结果：✅ 编译成功
```

### 影响

**架构优势：**
- ✅ 游戏逻辑层与渲染层彻底物理隔离
- ✅ 可在无 SFML 环境下编译（服务器、CI/CD）
- ✅ 轻松切换渲染后端（SFML/OpenGL/Vulkan）
- ✅ 跨平台兼容（MSVC/GCC/Clang）

**性能提升：**
- ✅ Vec2 和 Rect 都是 POD，可平凡复制
- ✅ 支持 constexpr，编译期计算
- ✅ 内联运算符，无虚函数开销

---

## [2026-03-29] - 功能新增 - HurtState & DeadState ✅

**类型：** 功能开发  
**优先级：** 🔴 高  
**影响范围：** 状态机系统

### 改动说明

实现受伤和死亡状态，完善状态机系统。

### 新增文件

- `include/states/HurtState.h` - 受伤状态（2 秒无敌）
- `include/states/DeadState.h` - 死亡状态（终态）
- `test/hurt_dead_state_test.cpp` - 单元测试（10 个测试）

### 修改文件

- `include/systems/StateMachineSystem.h` - 添加受伤和死亡转换逻辑
- `docs/04-Updates/CHANGELOG.md` - 更新改动记录

### 状态优先级

```
Dead > Hurt > Attack > Move > Idle
```

### 测试结果

```
✅ 10/10 测试通过
- HurtState 实例测试
- DeadState 实例测试
- 受伤状态转换
- 受伤期间不能移动
- 受伤期间不能攻击
- 死亡状态转换
- 死亡是终态
- 死亡优先级最高
- 受伤优先级高于攻击
- 状态优先级验证
```

---

## [2026-03-29] - 测试框架 - L0-L3 分层测试 ✅

**类型：** 测试框架  
**优先级：** 🟡 中  
**影响范围：** 测试基础设施

### 改动说明

建立分层测试体系，从架构验证到集成测试。

### 测试分层

| 层级 | 名称 | 测试内容 | 数量 |
|------|------|----------|------|
| **L0** | Paradigm & Memory | POD 验证、无虚函数、无依赖 | 3 |
| **L1** | Factory Assembly | 完整性装配、初始值验证 | 2 |
| **L2** | Isolated Logic | Movement、Modifier | 2 |
| **L3** | Pipeline & Tag | 战斗闭环、无敌拦截 | 2 |

### 新增文件

- `test/test_principles.h` - 测试原则宏定义
- `test/test_architecture.cpp` - 架构测试实现
- `docs/TEST_PRINCIPLES.md` - 测试原则文档

---

## [2026-03-29] - 功能新增 - 状态机基础框架 ✅

**类型：** 功能开发  
**优先级：** 🔴 高  
**影响范围：** 核心系统

### 改动说明

实现完整的状态机系统，包括 Idle、Move、Attack 状态。

### 新增文件

- `include/states/IState.h` - 状态接口
- `include/states/IdleState.h` - 待机状态
- `include/states/MoveState.h` - 移动状态
- `include/states/AttackState.h` - 攻击状态
- `include/components/StateMachine.h` - 状态机组件
- `include/systems/StateMachineSystem.h` - 状态机系统
- `test/state_machine_test.cpp` - 单元测试（8 个测试）

### 测试结果

```
✅ 8/8 测试通过
- 状态机初始化
- Idle→Move 转换
- Move→Idle 转换
- Idle→Attack 转换
- 攻击持续时间
- 攻击期间不能移动
- 状态名称和类型
- 单例模式
```

---

## [2026-03-29] - 功能新增 - 碰撞检测系统 ✅

**类型：** 功能开发  
**优先级：** 🔴 高  
**影响范围：** 战斗系统

### 改动说明

实现 Hitbox/Hurtbox 碰撞检测系统。

### 新增文件

- `include/utils/Rect.h` - 矩形和圆形碰撞体
- `include/utils/CollisionTypes.h` - 碰撞类型定义
- `include/components/Hitbox.h` - 攻击盒组件
- `include/components/Hurtbox.h` - 受击盒组件
- `include/systems/CollisionSystem.h` - 碰撞检测系统
- `test/collision_test.cpp` - 单元测试（6 个测试）

### 测试结果

```
✅ 6/6 测试通过
- 基本碰撞检测
- 无碰撞情况
- 无敌状态
- 圆形碰撞
- 圆形 vs 矩形
- 完整战斗流程
```

---

## [2026-03-29] - 文档重构 - 新文档结构 ✅

**类型：** 文档重构  
**优先级：** 🟢 低  
**影响范围：** 文档组织

### 新结构

```
docs/
├── 00_ARCHITECTURE.md         # 架构地图
├── 01_DATA_SCHEMA.md          # 数据字典
├── CHANGELOG.md               # 改动记录
├── CODE_REVIEW.md             # 代码审查
├── COMPONENT_LIST.md          # 组件清单
├── TEST_PRINCIPLES.md         # 测试原则
├── prompt.md                  # 全局原则（根目录）
├── features/                  # 功能模块
│   ├── F001-StateMachine.md
│   ├── F103-Attack.md
│   ├── F104-Hurt.md
│   ├── F105-Death.md
│   └── F201-Collision.md
└── journal/                   # 开发日志
```

---

**维护者:** Project2 Team  
**最后更新:** 2026-03-29  
**下次更新:** 每次功能提交前

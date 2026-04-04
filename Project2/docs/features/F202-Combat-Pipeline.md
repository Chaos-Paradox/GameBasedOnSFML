# F202 - 战斗管线 (Combat Pipeline)

**状态：** ✅ 已完成  
**日期：** 2026-03-29  
**优先级：** 🔴 高

---

## 🎯 功能概述

实现从"发起攻击"到"目标受击"的完整闭环，包含：
- 输入桥接（J 键攻击）
- 状态机升级（Attack/Hurt 状态）
- 攻击判定（Hitbox 临时实体）
- 碰撞检测（AABB）
- 伤害结算（DamageTag 单帧传递）
- 清理系统（自动销毁过期实体）

---

## 🏗️ 架构设计

### 系统执行顺序（精确到帧）

| 顺序 | System | 职责 | 输入 | 输出 |
|------|--------|------|------|------|
| 1 | InputSystem | 读取键盘 | SFML | InputCommand |
| 2 | StateMachineSystem | 状态切换 | InputCommand + **DamageTag** | State |
| 3 | DamageSystem | 伤害结算 | DamageTag | HP ↓ + 销毁 Tag |
| 4 | LocomotionSystem | 速度计算 | State + Input | Velocity |
| 5 | MovementSystem | 位置积分 | Velocity | Position |
| 6 | AttackSystem | 创建 Hitbox | State=Attack | **新实体** |
| 7 | CollisionSystem | 碰撞检测 | Hitbox + Hurtbox | **新 DamageTag** |
| 8 | CleanupSystem | 销毁过期 | Lifetime | 销毁实体 |

### 关键设计决策

1. **Hitbox 为独立临时实体**
   - 包含：Transform + HitboxComponent + LifetimeComponent
   - 存活 0.3 秒后自动销毁
   - 不挂载到玩家身上（符合 ECS 单一组件原则）

2. **DamageTag 单帧传递**
   - CollisionSystem 创建 → StateMachineSystem 读取 → DamageSystem 销毁
   - 防止一帧内多次扣血
   - 跨系统通信的唯一合法方式

3. **StateMachineSystem 读取 DamageTag**
   - 第 2 步读取，第 3 步销毁
   - 确保下一帧 StateMachine 能感知受击
   - Hurt 状态持续 0.5 秒

---

## 📦 新增 Component

### LifetimeComponent

```cpp
struct LifetimeComponent {
    float timeLeft{1.0f};    // 剩余存活时间
    bool autoDestroy{true};  // 时间到后自动销毁
};
```

### InputCommand（修改）

```cpp
struct InputCommand {
    Command cmd{Command::None};
    bool attackPressed{false};  // ← 新增：J 键攻击输入
};
```

---

## 🔧 新增 System

### AttackSystem

- 监听 Attack 状态
- 创建 Hitbox 临时实体
- 位置 = 玩家位置 + 前方 50 像素
- 存活 0.3 秒

### CollisionSystem

- AABB 碰撞检测
- 阵营检测（敌对才伤害）
- 命中历史（防止重复伤害）
- 挂载 DamageTag

### DamageSystem

- 扣减 currentHP
- **立即销毁 DamageTag**（关键！）

### CleanupSystem

- 减少 LifetimeComponent.timeLeft
- 销毁 timeLeft <= 0 的实体

---

## 🎮 沙盒测试

### VisualSandbox.cpp

**控制：**
- **WASD** - 移动
- **J** - 攻击（创建黄色 Hitbox）
- **ESC** - 退出

**视觉反馈：**
- 🟢 绿色方块 - 玩家
- 🔴 红色方块 - 假人（敌人）
- 🟡 黄色半透明 - Hitbox（攻击范围）
- 🔴 闪烁红色 - Hurt 状态

**控制台输出：**
```
Player: IDLE | HP: 100 | Hitboxes: 0
Player: ATTACK | HP: 100 | Hitboxes: 1
Player: HURT | HP: 90 | Hitboxes: 0
```

---

## 📊 测试验证

| 测试项 | 状态 | 说明 |
|--------|------|------|
| J 键创建 Hitbox | ✅ | 黄色方块显示 |
| Hitbox 0.3 秒销毁 | ✅ | CleanupSystem 工作 |
| 碰撞检测 | ✅ | AABB 检测正常 |
| DamageTag 传递 | ✅ | 单帧传递，无重复伤害 |
| Hurt 状态切换 | ✅ | 受击后闪烁红色 |
| HP 扣减 | ✅ | DamageSystem 工作 |

---

## ⚠️ 架构底线验证

| 底线 | 验证 | 状态 |
|------|------|------|
| 禁止直接扣血 | 通过 DamageTag 传递 | ✅ |
| 禁止状态机做碰撞 | CollisionSystem 独立 | ✅ |
| 禁止渲染越界 | SFML 只在沙盒 | ✅ |
| 禁止高频内存分配 | Hitbox 用 createEntity | ✅ |
| Tag 单帧销毁 | DamageSystem 立即销毁 | ✅ |
| Hitbox 独立实体 | 不挂载到玩家 | ✅ |

---

## 📝 文件清单

### 新增文件

| 文件 | 行数 | 说明 |
|------|------|------|
| `include/components/Lifetime.h` | 13 | 生命周期组件 |
| `include/systems/AttackSystem.h` | 71 | 攻击判定系统 |
| `include/systems/CollisionSystem.h` | 95 | 碰撞检测系统 |
| `include/systems/DamageSystem.h` | 35 | 伤害结算系统 |
| `include/systems/CleanupSystem.h` | 38 | 清理系统 |

### 修改文件

| 文件 | 修改 | 说明 |
|------|------|------|
| `include/components/InputCommand.h` | 新增 attackPressed | J 键输入 |
| `include/components/DamageTag.h` | 修复 EntityId → Entity | 类型统一 |
| `include/systems/StateMachineSystem.h` | 新增 DamageTag 读取 | 受击检测 |
| `tests/sandbox/VisualSandbox.cpp` | 完整战斗管线 | 沙盒测试 |

---

## 🎯 下一步扩展

### 短期
- [ ] 多段攻击（Combo）
- [ ] 不同攻击类型（物理/火焰/毒素）
- [ ] 击退效果
- [ ] 无敌帧机制

### 中期
- [ ] 远程攻击（投射物）
- [ ] 范围技能（AOE）
- [ ] 防御/格挡系统
- [ ] 连招系统

### 长期
- [ ] AI 战斗逻辑
- [ ] 多人同步
- [ ] 战斗动画状态机
- [ ] 战斗音效

---

**维护者：** Project2 Team  
**最后更新：** 2026-03-29  
**下次审查：** 添加新攻击类型时

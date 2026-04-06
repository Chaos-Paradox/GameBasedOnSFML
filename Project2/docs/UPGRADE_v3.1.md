# Upgrade Guide - v3.0 → v3.1 单轨覆盖指令槽

**发布日期：** 2026-04-06  
**Git 提交：** `880c567`  
**分支：** `develop`

---

## 🎯 升级概述

v3.1 版本将输入缓存架构从**限时独立缓存**升级为**单轨覆盖指令槽**，解决了长硬直场景下指令过期的致命问题。

### 版本对比

| 特性 | v3.0 (旧) | v3.1 (新) |
|------|-----------|-----------|
| **缓存方式** | `attackBufferTimer` 独立计时器 | `pendingIntent + intentTimer` 统一槽 |
| **指令冲突** | 多 Timer 并行，可能死锁 | Last-In-Wins，以最后按下为准 |
| **硬直场景** | 计时器持续倒计时，会过期 | 时间静止，指令永远保鲜 |
| **扩展性** | 每新增动作需加新 Timer | 只需扩展 `ActionIntent` 枚举 |

---

## 🔧 核心改动

### 1️⃣ InputCommand.h - 单轨指令槽

**v3.0 (旧):**
```cpp
struct InputCommand {
    Vec2 moveDir{0.0f, 0.0f};
    float attackBufferTimer{0.0f};  // ← 独立计时器
};
```

**v3.1 (新):**
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

---

### 2️⃣ StateMachineSystem.h - 时间静止魔法

**新增逻辑：**
```cpp
// 【时间静止魔法】僵直期间暂停 intentTimer 倒计时
if (input.intentTimer > 0.0f) {
    // 只在可行动状态下倒计时（Hurt/Dead/Dash 期间暂停）
    if (state.currentState != CharacterState::Hurt &&
        state.currentState != CharacterState::Dead &&
        state.currentState != CharacterState::Dash) {
        input.intentTimer -= dt;
    }
}
```

**效果：**
- 玩家在挨打第 0.1 秒按下冲刺
- 硬直 1.0 秒期间，`intentTimer` 永远冻结在 0.19 秒
- 硬直结束瞬间，指令依然有效，立即执行冲刺

---

### 3️⃣ VisualSandbox.cpp - Last-In-Wins 输入

**v3.0 (旧):**
```cpp
if (currentJPressed && !lastJPressed) {
    inputs.get(player).attackBufferTimer = 0.2f;
}
```

**v3.1 (新):**
```cpp
if (currentJPressed && !lastJPressed) {
    inputs.get(player).pendingIntent = ActionIntent::Attack;
    inputs.get(player).intentTimer = 0.2f;
}

if (currentSpacePressed && !lastSpacePressed) {
    inputs.get(player).pendingIntent = ActionIntent::Dash;  // ← 覆盖攻击
    inputs.get(player).intentTimer = 0.2f;                  // ← 重置保质期
}
```

---

### 4️⃣ DashSystem.h - 从指令槽读取意图

**v3.0 (旧):**
```cpp
void update(..., bool dashPressed, float dt)
```

**v3.1 (新):**
```cpp
void update(..., ComponentStore<InputCommand>& inputs, float dt)

// 内部逻辑
bool hasDashIntent = inputs.has(entity) && 
                     inputs.get(entity).pendingIntent == ActionIntent::Dash &&
                     inputs.get(entity).intentTimer > 0.0f;
```

---

## 🎮 实机测试验证

### 测试场景 1：长硬直指令保鲜

1. 启动 `VisualSandbox`
2. 右键生成假人沙袋
3. 攻击假人，触发受伤硬直（0.5 秒 Hurt）
4. **在硬直期间按下空格**
5. 硬直结束后，角色**立即冲刺**（不会呆立原地）

**预期结果：** ✅ 指令保鲜成功

---

### 测试场景 2：并发按键 Last-In-Wins

1. 启动 `VisualSandbox`
2. **同时按下 J + 空格**（先 J 后 Space）
3. 角色执行**冲刺**（而不是攻击）

**预期结果：** ✅ 以最后按下的按键为准

---

### 测试场景 3：攻击取消窗口

1. 启动 `VisualSandbox`
2. 按下 J 开始攻击
3. 攻击命中后 0.05 秒内按下 WASD
4. 攻击后摇被取消，立即恢复移动

**预期结果：** ✅ Cancel Window 正常工作

---

## 📝 迁移指南（如有其他 System 需要适配）

### 检查清单

- [ ] 搜索代码中的 `attackBufferTimer` 引用
- [ ] 更新所有 InputCommand 初始化（改为 `ActionIntent::None, 0.0f`）
- [ ] 更新所有输入检测逻辑（改为设置 `pendingIntent + intentTimer`）
- [ ] 更新所有消费逻辑（改为检查 `pendingIntent == ActionIntent::X`）

### 初始化模板

```cpp
// 创建玩家时
inputs.add(player, {
    .moveDir = {0.0f, 0.0f},
    .pendingIntent = ActionIntent::None,
    .intentTimer = 0.0f
});
```

### 输入录入模板

```cpp
// 在每帧输入检测时
if (keyPressed && !lastKeyPressed) {
    inputs.get(player).pendingIntent = ActionIntent::Attack;  // 或 Dash
    inputs.get(player).intentTimer = 0.2f;
}
```

### 意图消费模板

```cpp
// 在 System 中消费意图
if (inputs.has(entity) && 
    inputs.get(entity).pendingIntent == ActionIntent::Attack &&
    inputs.get(entity).intentTimer > 0.0f) {
    
    // 执行动作...
    
    // 精准消费：清零意图
    inputs.get(entity).pendingIntent = ActionIntent::None;
    inputs.get(entity).intentTimer = 0.0f;
}
```

---

## 🐛 已知问题

暂无

---

## 📚 相关文档

- [`00_ARCHITECTURE.md`](00_ARCHITECTURE.md) - 架构地图（已更新 v3.1）
- [`features/F101-Input.md`](features/F101-Input.md) - 输入系统详解
- [`../include/components/InputCommand.h`](../include/components/InputCommand.h) - 组件定义
- [`../include/systems/StateMachineSystem.h`](../include/systems/StateMachineSystem.h) - 状态机实现

---

## 🚀 回滚方案（如有必要）

如需回滚到 v3.0：

```bash
git revert 880c567
# 或
git reset --hard 7baf636
```

---

**升级完成！** 🎉

如有问题，请在 GitHub 提交 Issue 或在 Discord 社区讨论。

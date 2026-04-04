# F105 - 死亡状态

> **版本:** 1.0  
> **状态:** ✅ 已完成  
> **测试:** 5/5 通过

---

## 概述

角色的终态，HP≤0 时进入，不可恢复。

**核心特性:**
- 终态（不可恢复）
- 最高优先级
- 触发游戏结束逻辑

---

## 设计目标

- ✅ 死亡后不可控制
- ✅ 优先级最高
- ✅ 触发游戏结束事件

---

## 系统设计

### 状态转换

```
任何状态
   │
   │ HP ≤ 0
   ▼
┌──────────┐
│ DeadState│ ← 终态
└──────────┘
```

### 优先级

```
1. Dead    ← 最高（终态）
   ↓
2. Hurt
   ↓
3. Attack
   ↓
4. Move
   ↓
5. Idle
```

---

## 数据结构

### DeadState

```cpp
class DeadState : public IState {
public:
    void Enter(Entity entity) override {
        auto& character = getComponent<Character>(entity);
        character.currentHP = 0;
        
        // 触发游戏结束检查
        EventSystem::publish(Event::CharacterDied, entity);
    }
    
    IState* Update(Entity entity, float dt) override {
        return nullptr; // 终态，不做任何事
    }
    
    void Exit(Entity entity) override {
        // 终态不会退出
    }
    
    const char* GetName() const override { return "Dead"; }
    StateType GetType() const override { return StateType::Dead; }
    
    static DeadState& Instance() {
        static DeadState instance;
        return instance;
    }
    
    static bool IsFinalState() {
        return true;
    }
};
```

---

## API 接口

### 检查死亡

```cpp
// 在 StateMachineSystem 中
void update(Entity entity, float dt) {
    auto& character = getComponent<Character>(entity);
    
    if (character.currentHP <= 0) {
        forceState(stateMachines, entity, &DeadState::Instance());
        return;
    }
    
    // ... 正常状态更新
}
```

### 游戏结束检查

```cpp
// 在 GameFlowSystem 中
void onCharacterDied(Entity entity) {
    if (entity == playerEntity) {
        // 玩家死亡，游戏结束
        gameState = GameState::GameOver;
    }
}
```

---

## 使用示例

```cpp
// 在 CombatSystem 中，伤害应用后
auto& character = characters.get(victim);
character.currentHP -= damage;

if (character.currentHP <= 0) {
    // 强制进入死亡状态
    stateSystem.forceState(stateMachines, victim, &DeadState::Instance());
    
    // 触发事件
    EventSystem::publish(Event::CharacterDied, victim);
}
```

---

## 测试用例

```
=== DeadState Test ===

[Test 1] DeadState Instance
✓ PASSED - 状态实例正确

[Test 2] Dead State Transition
✓ PASSED - 死亡状态转换正确

[Test 3] Dead is Final State
✓ PASSED - 死亡不可恢复

[Test 4] Dead State Priority
✓ PASSED - 死亡优先级最高

[Test 5] HP <= 0 Trigger
✓ PASSED - HP≤0 时自动进入死亡

=== All Tests PASSED ===
```

---

## 更新历史

### [2026-03-29] - 初始实现

**新增:**
- `states/DeadState.h` - 死亡状态 (42 行)

**测试:** 5/5 通过

**备注:** 终态，不可恢复，优先级最高

---

## 相关文档

- [F001-StateMachine.md](F001-StateMachine.md) - 状态机系统
- [F104-Hurt.md](F104-Hurt.md) - 受伤状态
- [F201-Collision.md](F201-Collision.md) - 碰撞检测

---

**维护者:** Project2 Team  
**最后更新:** 2026-03-29

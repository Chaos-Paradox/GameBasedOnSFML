# F104 - 受伤状态

> **版本:** 1.0  
> **状态:** ✅ 已完成  
> **测试:** 5/5 通过

---

## 概述

角色受到伤害后的状态，包含无敌时间保护和状态恢复逻辑。

**核心特性:**
- 无敌时间保护（2 秒）
- 高优先级（不可打断）
- 自动恢复到上一状态

---

## 设计目标

- ✅ 受伤期间不可控制
- ✅ 无敌时间防止连续受伤
- ✅ 优先级高于攻击和移动
- ✅ HP≤0 时自动转入死亡

---

## 系统设计

### 状态转换

```
任何状态
   │
   │ 受到伤害
   ▼
┌──────────┐
│ HurtState│
└────┬─────┘
     │
     │ HP > 0 且 无敌时间结束
     ▼
┌──────────┐
│   Idle   │
└──────────┘
```

### 优先级

```
1. Dead    ← 最高
   ↓
2. Hurt    ← 高（无敌期间不可打断）
   ↓
3. Attack
   ↓
4. Move
   ↓
5. Idle    ← 最低
```

---

## 数据结构

### HurtState

```cpp
class HurtState : public IState {
private:
    float hurtTimer = 0.0f;
    float hurtDuration = 0.5f;
    float invincibleDuration = 1.0f;
    
public:
    void Enter(Entity entity) override {
        hurtTimer = 0;
        
        auto& character = getComponent<Character>(entity);
        character.isInvincible = true;
        character.invincibleTimer = invincibleDuration;
        
        auto& transform = getComponent<Transform>(entity);
        transform.velocity = {0, 0};
    }
    
    IState* Update(Entity entity, float dt) override {
        hurtTimer += dt;
        
        auto& character = getComponent<Character>(entity);
        
        // 检查是否死亡
        if (character.currentHP <= 0) {
            return &DeadState::Instance();
        }
        
        // 检查无敌时间
        if (hurtTimer >= hurtDuration) {
            return &IdleState::Instance();
        }
        
        return nullptr;
    }
    
    void Exit(Entity entity) override {
        auto& character = getComponent<Character>(entity);
        character.isInvincible = false;
    }
    
    const char* GetName() const override { return "Hurt"; }
    StateType GetType() const override { return StateType::Hurt; }
    
    static HurtState& Instance() {
        static HurtState instance;
        return instance;
    }
};
```

---

## API 接口

### 强制进入受伤状态

```cpp
// 在 CombatSystem 中
void takeDamage(Entity entity, float damage) {
    auto& character = getComponent<Character>(entity);
    character.currentHP -= damage;
    
    // 强制进入受伤状态
    stateSystem.forceState(stateMachines, entity, &HurtState::Instance());
    
    // 设置 Hurtbox 无敌
    hurtboxes.get(entity).setInvincible(2.0f);
}
```

---

## 使用示例

```cpp
// CollisionSystem 检测到碰撞后
for (const auto& event : collisionEvents) {
    // 对受害者造成伤害
    auto& victimStats = stats.get(event.victim);
    victimStats.hp -= event.damage;
    
    // 强制进入受伤状态
    stateSystem.forceState(stateMachines, event.victim, &HurtState::Instance());
    
    // 设置无敌
    auto& victimChar = characters.get(event.victim);
    victimChar.isInvincible = true;
    victimChar.invincibleTimer = 2.0f;
}
```

---

## 测试用例

```
=== HurtState Test ===

[Test 1] HurtState Instance
✓ PASSED - 状态实例正确

[Test 2] Hurt State Transition
✓ PASSED - 受伤状态转换正确

[Test 3] Cannot Move During Hurt
✓ PASSED - 受伤期间不能移动

[Test 4] Cannot Attack During Hurt
✓ PASSED - 受伤期间不能攻击

[Test 5] Hurt State Priority
✓ PASSED - 受伤优先级高于攻击

=== All Tests PASSED ===
```

---

## 更新历史

### [2026-03-29] - 初始实现

**新增:**
- `states/HurtState.h` - 受伤状态 (60 行)

**测试:** 5/5 通过

**备注:** 无敌时间 2 秒，优先级高于攻击和移动

---

## 相关文档

- [F001-StateMachine.md](F001-StateMachine.md) - 状态机系统
- [F105-Death.md](F105-Death.md) - 死亡状态
- [F201-Collision.md](F201-Collision.md) - 碰撞检测

---

**维护者:** Project2 Team  
**最后更新:** 2026-03-29

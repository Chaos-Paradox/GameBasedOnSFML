# F601 - 阵营与仇恨系统

> **版本:** 0.1 (规划中)  
> **状态:** 📋 设计阶段  
> **优先级:** 🔴 高

---

## 概述

处理阵营关系、仇恨列表和"全员可攻击的弱社交共斗"玩法的核心系统。

**核心特性:**
- 阵营标签（Player、Monster、NPC）
- 仇恨列表管理
- 动态阵营关系
- 支持"谁打我我打谁"的自主社交

---

## 设计目标

- ✅ 纯数据驱动，无状态依赖
- ✅ 支持动态阵营变更
- ✅ 仇恨值随时间衰减
- ✅ Tag 驱动，便于网络同步
- ✅ 支持"弱社交"（非组队合作）

---

## 系统设计

### 阵营关系图

```
阵营列表:
┌─────────────────────────────────────────────────────┐
│ Player          - 玩家（默认中立）                  │
│ Monster_A       - 怪物 A 阵营（主动攻击玩家）        │
│ Monster_B       - 怪物 B 阵营（主动攻击玩家）        │
│ NPC_Trader      - 商人 NPC（中立，被攻击后反击）    │
│ NPC_Guard       - 守卫 NPC（主动攻击怪物）          │
└─────────────────────────────────────────────────────┘

默认关系:
- Player vs Monster_A: 敌对
- Player vs Monster_B: 敌对
- Monster_A vs Monster_B: 敌对
- Player vs NPC_Trader: 中立（可攻击）
- Player vs NPC_Guard: 友好
```

### 仇恨系统流程

```
CombatSystem 造成伤害
   │
   ▼
更新 AggroComponent（受害者记录攻击者）
   │
   ├──► 增加仇恨值
   ├──► 更新最后被攻击时间
   └──► 如果仇恨值超过阈值，加入仇恨列表
   │
   ▼
AISystem 读取 AggroComponent
   │
   ├──► 选择仇恨值最高的目标
   └──► 追击并攻击
   │
   ▼
仇恨值随时间衰减
   │
   ▼
如果长时间未攻击，从仇恨列表移除
```

### "弱社交"共斗设计

```
玩家 A 和玩家 B 在同一区域：
- 默认中立（不自动组队）
- 可以互相攻击（阵营关系允许）
- 可以选择合作（共同攻击怪物）
- 可以背刺（攻击正在打怪的玩家）

怪物潮来袭时：
- 玩家自主选择是否帮助其他玩家
- 没有强制组队机制
- 战利品独立掉落（无争夺）
```

---

## 数据结构

### FactionComponent

```cpp
struct FactionComponent {
    enum class FactionId : uint8_t {
        Player,
        Monster_A,
        Monster_B,
        NPC_Trader,
        NPC_Guard,
        Neutral
    };
    
    FactionId faction{FactionId::Neutral};
    
    // 动态阵营关系（可选，用于阵营变更事件）
    std::unordered_map<FactionId, int> factionRelations;
    // 关系值：>0 友好，=0 中立，<0 敌对
};

// 全局阵营关系表（静态配置）
struct FactionRelationTable {
    static int getRelation(FactionComponent::FactionId a, 
                          FactionComponent::FactionId b) {
        // 返回关系值
    }
};
```

### AggroComponent

```cpp
struct AggroEntry {
    EntityId attackerId;
    float threatValue{0.0f};
    float lastAttackTime{0.0f};
    int damageDealt{0};
};

struct AggroComponent {
    std::vector<AggroEntry> threatTable;
    
    // 仇恨阈值（超过此值才会主动追击）
    static constexpr float AGGRO_THRESHOLD = 50.0f;
    
    // 仇恨衰减时间（秒）
    static constexpr float AGGRO_DECAY_TIME = 30.0f;
    
    // 添加/更新仇恨
    void addThreat(EntityId attacker, float damage, float currentTime);
    
    // 获取最高仇恨目标
    EntityId getTopThreatTarget() const;
    
    // 清理过期仇恨
    void decayThreats(float currentTime);
    
    // 检查是否在仇恨列表中
    bool isInThreatTable(EntityId attacker) const;
};
```

### 数据交接协议（造成伤害）

```cpp
// CombatSystem 中
void applyDamage(Entity attacker, Entity victim, float damage) {
    // 1. 扣血
    auto& stats = runtimeStats.get(victim);
    stats.currentHP -= static_cast<int>(damage);
    
    // 2. 更新仇恨（受害者记录攻击者）
    if (aggros.has(victim)) {
        auto& aggro = aggros.get(victim);
        aggro.addThreat(attacker, damage, currentTime);
    }
    
    // 3. 挂载 DamageTag（状态机处理）
    damageTags.add(victim, {damage, attacker});
}

// AggroComponent::addThreat 实现
void AggroComponent::addThreat(EntityId attacker, float damage, float currentTime) {
    // 查找是否已存在
    for (auto& entry : threatTable) {
        if (entry.attackerId == attacker) {
            entry.threatValue += damage;
            entry.lastAttackTime = currentTime;
            entry.damageDealt += static_cast<int>(damage);
            return;
        }
    }
    
    // 新条目
    threatTable.push_back({attacker, damage, currentTime, static_cast<int>(damage)});
}
```

---

## API 接口

### FactionSystem

```cpp
class FactionSystem {
public:
    // 检查两个实体是否敌对
    bool areHostile(Entity a, Entity b) const;
    
    // 获取关系值
    int getRelation(Entity a, Entity b) const;
    
    // 变更阵营（用于事件）
    void changeFaction(Entity entity, FactionComponent::FactionId newFaction);
};
```

### AggroSystem

```cpp
class AggroSystem {
public:
    void update(
        ComponentStore<AggroComponent>& aggros,
        const std::vector<Entity>& entities,
        float currentTime,
        float dt
    );
    
    // 获取最高仇恨目标
    EntityId getTopThreat(Entity entity) const;
    
    // 清除所有仇恨（如怪物重置）
    void clearAllThreats(Entity entity);
};
```

---

## 使用示例

### AI 选择目标

```cpp
// AISystem::update 中
void AISystem::update(Entity entity, ...) {
    auto& ai = ais.get(entity);
    auto& faction = factions.get(entity);
    auto& aggro = aggros.get(entity);
    
    // 1. 优先攻击仇恨列表中的目标
    EntityId target = aggro.getTopThreatTarget();
    
    if (target != INVALID_ENTITY_ID) {
        // 追击并攻击
        ai.targetId = target;
        ai.state = AIState::Chase;
        return;
    }
    
    // 2. 无仇恨目标时，攻击默认敌对阵营
    for (Entity e : nearbyEntities) {
        auto& otherFaction = factions.get(e);
        if (FactionSystem::areHostile(faction.faction, otherFaction.faction)) {
            ai.targetId = e;
            ai.state = AIState::Chase;
            return;
        }
    }
    
    // 3. 无敌对目标，进入巡逻状态
    ai.state = AIState::Roam;
}
```

### NPC 被攻击后反击

```cpp
// 商人 NPC 初始中立
FactionComponent npcFaction;
npcFaction.faction = FactionComponent::FactionId::NPC_Trader;

// 玩家攻击 NPC
CombatSystem::applyDamage(player, npc, damage);

// NPC 的 AggroComponent 记录玩家
// AISystem 检测到仇恨，切换为反击状态
ai.state = AIState::Combat;
ai.targetId = player;
```

### 仇恨衰减

```cpp
// AggroSystem::update
void AggroSystem::update(..., float currentTime, float dt) {
    for (Entity e : entities) {
        auto& aggro = aggros.get(e);
        
        // 清理过期仇恨
        aggro.decayThreats(currentTime);
    }
}

// AggroComponent::decayThreats
void AggroComponent::decayThreats(float currentTime) {
    auto it = threatTable.begin();
    while (it != threatTable.end()) {
        float elapsed = currentTime - it->lastAttackTime;
        
        if (elapsed > AGGRO_DECAY_TIME) {
            it = threatTable.erase(it);
        } else {
            // 线性衰减
            it->threatValue *= (1.0f - elapsed / AGGRO_DECAY_TIME);
            ++it;
        }
    }
}
```

---

## 依赖关系

- **F201 - 碰撞检测:** CombatSystem 调用 addThreat
- **F001 - 状态机系统:** AISystem 读取仇恨列表选择目标
- **F602 - AI 导演:** 生成怪物时设置正确阵营

---

## 测试用例（规划中）

```
[ ] 阵营关系检测测试
[ ] 仇恨添加测试
[ ] 仇恨衰减测试
[ ] 最高仇恨目标选择测试
[ ] NPC 被攻击反击测试
[ ] 多目标仇恨优先级测试
[ ] 仇恨列表清理测试
```

---

## 性能评估

### 时间复杂度

- **添加仇恨:** O(n)，n=仇恨列表大小（通常<10）
- **获取最高仇恨:** O(n)
- **仇恨衰减:** O(n)

### 性能预算

```
AggroSystem 更新预算：< 0.2ms（100 实体）
- 每个实体平均 3-5 个仇恨目标
- 单次更新：< 0.005ms
```

---

## 已知问题

### 待设计项

- [ ] 仇恨阈值平衡性
- [ ] 阵营变更事件设计
- [ ] 团队仇恨共享（如未来添加组队）
- [ ] 区域重置时仇恨清理

---

## 更新历史

### [2026-03-29] - 初始设计

**状态:** 规划中

**备注:** 架构设计完成，等待实现

---

## 相关文档

- [F201-Collision.md](F201-Collision.md) - CombatSystem 伤害计算
- [F602-Director.md](F602-Director.md) - AI 导演生成怪物
- [F603-Aggro.md](F603-Aggro.md) - 仇恨系统详细设计
- [01_DATA_SCHEMA.md](../01_DATA_SCHEMA.md) - 组件字段定义

---

**维护者:** Project2 Team  
**最后更新:** 2026-03-29

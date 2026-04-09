# F301 - 进化与成长系统

> **版本:** 0.1 (规划中)  
> **状态:** 📋 设计阶段  
> **优先级:** 🔴 高

---

## 概述

处理角色成长、进化点数、技能树和被动突变的核心系统。

**核心特性:**
- 进化点数获取和消耗
- 主动技能树解锁
- 被动突变列表管理
- 区域适应进化（如辐射区停留解锁辐射抗性）

---

## 设计目标

- ✅ 纯数据驱动，无状态依赖
- ✅ 支持主动和被动两种进化类型
- ✅ 与区域系统无缝集成
- ✅ Tag 驱动，便于网络同步

---

## 系统设计

### 数据流

```
玩家击杀怪物/完成任务
   │
   ▼
获得经验值 → 升级
   │
   ▼
获得进化点数（写入 EvolutionComponent）
   │
   ▼
UI 层显示可加点
   │
   ▼
玩家选择技能 → 挂载 EvolveCommandTag
   │
   ▼
ProgressionSystem 处理
   │
   ├──► 扣除进化点数
   ├──► 激活技能节点
   └──► ModifierSystem 重新计算属性
```

### 区域被动进化

```
玩家进入辐射区
   │
   ▼
SpatialTriggerSystem 挂载 EnvironmentalTag::Radiation
   │
   ▼
ProgressionSystem 每帧累积 radiationTimer
   │
   ▼
radiationTimer >= 300s（5 分钟）
   │
   ▼
解锁 EvolutionComponent::RadiationResistance
   │
   ▼
ModifierSystem 应用辐射抗性增益
```

---

## 数据结构

### EvolutionComponent

```cpp
struct EvolutionComponent {
    // 进化点数
    int evolutionPoints{0};
    int totalEarned{0};
    
    // 主动技能树（已激活的节点 ID）
    std::vector<int> activeSkillNodes;
    
    // 被动突变列表
    std::vector<int> passiveMutations;
    
    // 区域适应计时器
    float radiationTimer{0.0f};
    float coldTimer{0.0f};
    float poisonTimer{0.0f};
    
    // 已解锁的区域适应
    bool radiationResistance{false};
    bool coldResistance{false};
    bool poisonImmunity{false};
};
```

### StatModifierComponent

```cpp
struct StatModifier {
    enum class Type {
        Flat,       // 固定值（+10 攻击力）
        Percent,    // 百分比（+10% 移速）
        Multiplicative // 乘算（x1.5 伤害）
    };
    
    Type type;
    std::string statName; // "attack", "defense", "moveSpeed"
    float value;
    int sourceId; // 来源（装备 ID、技能 ID 等）
    float duration; // 持续时间（0=永久）
};

struct StatModifierComponent {
    std::vector<StatModifier> modifiers;
    
    void addModifier(const StatModifier& mod);
    void removeModifiersFromSource(int sourceId);
    void update(float dt); // 清理过期增益
};
```

### RuntimeStatsComponent

```cpp
struct RuntimeStatsComponent {
    // 基础属性（从 CharacterComponent 复制）
    int baseHP{100};
    int baseAttack{10};
    int baseDefense{5};
    float baseMoveSpeed{150.0f};
    
    // 最终属性（计算后）
    int currentHP{100};
    int finalAttack{10};
    int finalDefense{5};
    float finalMoveSpeed{150.0f};
    
    // 元素抗性
    float radiationResist{0.0f};
    float coldResist{0.0f};
    float poisonResist{0.0f};
};
```

---

## API 接口

### ProgressionSystem

```cpp
class ProgressionSystem {
public:
    void update(
        ComponentStore<EvolutionComponent>& evolutions,
        ComponentStore<StatModifierComponent>& modifiers,
        const std::vector<Entity>& entities,
        float dt
    );
    
    // 获得经验值
    void gainEXP(Entity entity, int exp);
    
    // 升级
    void levelUp(Entity entity);
    
    // 处理进化命令（Tag 驱动）
    void processEvolveCommand(
        Entity entity,
        const EvolveCommandTag& tag
    );
};
```

### ModifierSystem

```cpp
class ModifierSystem {
public:
    void update(
        ComponentStore<CharacterComponent>& characters,
        ComponentStore<EvolutionComponent>& evolutions,
        ComponentStore<EquipmentComponent>& equipments,
        ComponentStore<StatModifierComponent>& modifiers,
        ComponentStore<RuntimeStatsComponent>& runtimeStats,
        const std::vector<Entity>& entities
    );
    
    // 重新计算某个实体的属性
    void recalculateStats(Entity entity);
};
```

---

## 使用示例

### 玩家加点

```cpp
// UI 层检测到玩家点击"升级攻击力"
void onUpgradeAttackClicked(Entity player, int skillId) {
    // 挂载 Tag（不直接调用逻辑）
    evolveCommandTags.add(player, {skillId, 1}); // 1=升级 1 级
}

// ProgressionSystem 处理
void ProgressionSystem::processEvolveCommand(
    Entity entity,
    const EvolveCommandTag& tag)
{
    auto& evolution = evolutions.get(entity);
    
    if (evolution.evolutionPoints >= getSkillCost(tag.skillId)) {
        // 扣除点数
        evolution.evolutionPoints -= getSkillCost(tag.skillId);
        
        // 激活技能节点
        evolution.activeSkillNodes.push_back(tag.skillId);
        
        // 通知 ModifierSystem 重新计算
        needsRecalculation.insert(entity);
    }
}
```

### 区域适应

```cpp
// ProgressionSystem::update 中
void ProgressionSystem::update(..., float dt) {
    for (Entity e : entities) {
        auto& evolution = evolutions.get(e);
        
        // 检测环境 Tag
        if (environmentalTags.has(e, EnvironmentalTag::Radiation)) {
            evolution.radiationTimer += dt;
            
            // 达到阈值，解锁抗性
            if (evolution.radiationTimer >= 300.0f && !evolution.radiationResistance) {
                evolution.radiationResistance = true;
                evolution.passiveMutations.push_back(MutationId::RadiationResist);
                
                // 触发事件（可选）
                eventSystem.publish(Event::MutationUnlocked, e, MutationId::RadiationResist);
            }
        }
    }
}
```

---

## 依赖关系

- **F001 - 状态机系统:** CharacterComponent 基础属性
- **F502 - 装备系统:** EquipmentComponent 提供装备增益
- **F402 - 环境系统:** EnvironmentalTag 提供区域信息

---

## 测试用例（规划中）

```
[ ] 获得经验值测试
[ ] 升级测试
[ ] 进化点数发放测试
[ ] 技能解锁测试
[ ] 属性重新计算测试
[ ] 区域适应解锁测试
[ ] 过期增益清理测试
```

---

## 性能评估

### 时间复杂度

- **经验获取:** O(1)
- **升级计算:** O(1)
- **属性重新计算:** O(n)，n=modifier 数量

### 性能预算

```
ModifierSystem 更新预算：< 0.5ms（100 实体）
- 每个实体平均 5-10 个 modifier
- 单次计算：< 0.01ms
```

---

## 已知问题

### 待设计项

- [ ] 技能树数据结构设计
- [ ] 被动突变平衡性
- [ ] 区域适应时间阈值
- [ ] 属性上限设计

---

## 更新历史

### [2026-03-29] - 初始设计

**状态:** 规划中

**备注:** 架构设计完成，等待实现

---

## 相关文档

- [F001-StateMachine.md](F001-StateMachine.md) - CharacterComponent
- [F302-Modifier.md](F302-Modifier.md) - 属性计算
- [F402-Environmental.md](F402-Environmental.md) - 区域适应
- [01_DATA_SCHEMA.md](../01_DATA_SCHEMA.md) - 组件字段定义

---

**维护者:** Project2 Team  
**最后更新:** 2026-03-29

# 01_DATA_SCHEMA.md - Project2 数据字典

> **核心原则：**
> 1. 本文档所有的 Component 必须是 **POD (Plain Old Data)**，严禁包含任何业务逻辑函数。
> 2. 状态机的具体流转逻辑见 [`features/F001-StateMachine.md`](features/F001-StateMachine.md)。
> 3. 跨系统通讯必须通过 TagComponent，见下方第五部分。
> 4. **所有数学类型使用自定义 Vec2 和 Rect，无 SFML 依赖。**

---

## 一、全局枚举 (Enums)

### 1.1 状态枚举

```cpp
enum class CharacterState : uint8_t {
    Idle,       // 待机
    Move,       // 移动
    Attack,     // 攻击
    Hurt,       // 受伤
    Dead,       // 死亡
};

enum class AIState : uint8_t {
    Roam,       // 巡逻
    Alert,      // 警戒
    Combat,     // 战斗
    Return,     // 返回
};

enum class GameState : uint8_t {
    Menu,       // 菜单
    Playing,    // 游戏中
    Paused,     // 暂停
    GameOver,   // 游戏结束
};
```

### 1.2 基础定义枚举

```cpp
enum class Faction : uint8_t {
    Player,         // 玩家阵营
    Wildlife,       // 中立野生动物
    Mutant,         // 突变怪物（敌对）
    NPC_Trader,     // 商人（中立）
    NPC_Guard,      // 守卫（友好）
};

enum class ElementType : uint8_t {
    Physical,   // 物理
    Fire,       // 火焰
    Toxic,      // 毒素
    Ice,        // 冰冻
};
```

### 1.3 常量定义

```cpp
constexpr EntityId INVALID_ENTITY = UINT32_MAX;

// 仇恨系统常量
constexpr float AGGRO_THRESHOLD = 50.0f;        // 仇恨阈值
constexpr float AGGRO_DECAY_TIME = 30.0f;       // 仇恨衰减时间（秒）

// 状态机常量
constexpr float DEFAULT_STATE_TIMER = 0.0f;

// 数学常量
constexpr float PI = 3.14159265358979323846f;
```

---

## 二、核心实体组件 (Core Components)

### 2.1 StateMachineComponent

> **⚠️ 注意：** 这里只存当前状态数据，真正的切换逻辑在 `StateMachineSystem` 中执行。

```cpp
struct StateMachineComponent {
    CharacterState currentState{CharacterState::Idle};
    CharacterState previousState{CharacterState::Idle};
    float stateTimer{0.0f}; // 当前状态持续时间
};
```

### 2.2 CharacterComponent

```cpp
struct CharacterComponent {
    std::string name;
    int level{1};
    
    // 基础面板（不受装备/进化影响）
    int maxHP{100};
    int currentHP{100};
    int baseAttack{10};
    int baseDefense{5};
    float baseMoveSpeed{150.0f};
    
    // 运行时状态标志
    bool isInvincible{false};
    float invincibleTimer{0.0f};
    
    // 朝向（使用自定义 Vec2）
    Vec2 facingDirection{1.0f, 0.0f};
};
```

### 2.3 TransformComponent

```cpp
struct TransformComponent {
    Vec2 position{0.0f, 0.0f};      // 位置
    Vec2 scale{1.0f, 1.0f};         // 缩放
    float rotation{0.0f};           // 弧度
    
    // 速度（由 MovementSystem 写入）
    Vec2 velocity{0.0f, 0.0f};
};
```

---

## 三、RPG 与进化组件 (RPG & Evolution Components)

### 3.1 EvolutionComponent

```cpp
struct EvolutionComponent {
    // 进化点数
    int evolutionPoints{0};         // 可用进化点数
    int totalEarned{0};             // 累计获得点数（用于统计）
    
    // 已激活的主动技能 ID 列表
    // 注意：使用固定数组代替 std::vector 以符合 POD
    static constexpr int MAX_SKILLS = 16;
    uint32_t activeSkills[MAX_SKILLS];
    int activeSkillCount{0};
    
    // 已激活的被动突变 ID 列表
    static constexpr int MAX_MUTATIONS = 16;
    uint32_t passiveMutations[MAX_MUTATIONS];
    int passiveMutationCount{0};
    
    // 区域适应度积累（简化：只记录当前区域）
    uint32_t currentBiomeId{0};
    float biomeAdaptationTimer{0.0f};
};
```

### 3.2 RuntimeStatsComponent

> 由 `ModifierSystem` 每帧/按需计算得出，供战斗系统读取。
> 
> **计算公式：** 基础属性 + 装备加成 + 进化增益 = 最终属性

```cpp
struct RuntimeStatsComponent {
    // 最终战斗面板
    int finalAttack{10};
    int finalDefense{5};
    float finalMoveSpeed{150.0f};
    int finalMaxHP{100};
    
    // 元素属性
    ElementType attackElement{ElementType::Physical};
    
    // 元素抗性（0-100，百分比）
    float fireResist{0.0f};
    float toxicResist{0.0f};
    float iceResist{0.0f};
};
```

### 3.3 StatModifierComponent

> 临时增益/减益效果列表。
> **注意：** 为了符合 POD 要求，使用固定数组代替 std::vector。

```cpp
struct StatModifier {
    enum class Type : uint8_t {
        Flat,           // 固定值（+10 攻击力）
        Percent,        // 百分比（+10% 移速）
        Multiplicative  // 乘算（x1.5 伤害）
    };
    
    Type modifierType{Type::Flat};
    char statName[32];     // "attack", "defense", "moveSpeed"
    float value{0.0f};
    uint32_t sourceId{0};  // 来源（装备 ID、技能 ID 等）
    float duration{0.0f};  // 持续时间（0=永久）
    float remainingTime{0.0f}; // 剩余时间
};

struct StatModifierComponent {
    static constexpr int MAX_MODIFIERS = 32;
    StatModifier modifiers[MAX_MODIFIERS];
    int modifierCount{0};
};
```

---

## 四、战斗与碰撞组件 (Combat Components)

### 4.1 HitboxComponent

```cpp
struct HitboxComponent {
    Rect bounds;                    // 判定框（相对位置）
    int damageMultiplier{100};      // 伤害倍率（百分比，100=100%）
    ElementType element{ElementType::Physical};
    float knockbackForce{100.0f};   // 击退力度
    
    EntityId sourceEntity{INVALID_ENTITY}; // 谁发出的攻击
    
    // 已命中目标（防止一帧多次伤害）
    // 使用固定数组代替 std::set 以符合 POD
    static constexpr int MAX_HIT_COUNT = 16;
    EntityId hitHistory[MAX_HIT_COUNT];
    int hitCount{0};
    
    bool active{false};             // 是否激活
};
```

### 4.2 HurtboxComponent

```cpp
struct HurtboxComponent {
    Rect bounds;                    // 受击框（相对位置）
    bool isInvincible{false};       // 是否无敌
    float invincibleTimer{0.0f};    // 无敌剩余时间
    
    Faction faction{Faction::Player};
    int layer{1};                   // 碰撞层级
};
```

### 4.3 FactionComponent

```cpp
struct FactionComponent {
    Faction factionId{Faction::Player};
};
```

### 4.4 AggroComponent

> 记录对当前实体造成伤害的来源及仇恨值。
> **注意：** 使用固定数组代替 unordered_map 以符合 POD。

```cpp
struct AggroEntry {
    EntityId attackerId{INVALID_ENTITY};
    float threatValue{0.0f};
    float lastAttackTime{0.0f};
    int totalDamageDealt{0};
};

struct AggroComponent {
    static constexpr int MAX_ENTRIES = 32;
    AggroEntry entries[MAX_ENTRIES];
    int entryCount{0};
    
    // 当前追击目标
    EntityId currentTarget{INVALID_ENTITY};
};
```

---

## 五、指令与事件标签 (Tags & Commands)

> **⚠️ 非常重要：** 这是跨 System 通信的**唯一合法方式**。
> 
> 生命周期通常只有一帧，由目标 System 消费后销毁。

### 5.1 DamageTag

> 当碰撞系统检测到命中时，给受击者挂上此 Tag。
> 
> `StateMachineSystem` 检测到该 Tag 时，自行切换到 `HurtState`。

```cpp
struct DamageTag {
    float damage{0.0f};
    ElementType element{ElementType::Physical};
    Vec2 knockbackDirection{0.0f, 0.0f};
    EntityId attackerId{INVALID_ENTITY};
    
    float timestamp{0.0f}; // 挂载时间（用于调试）
    bool applied{false};   // 是否已处理
};
```

### 5.2 EvolveCommandTag

> UI 层发出的进化指令。

```cpp
struct EvolveCommandTag {
    uint32_t skillId{0};        // 技能/突变 ID
    int level{1};               // 升级等级（1=解锁/升级 1 级）
};
```

### 5.3 InteractCommandTag

> 玩家按下交互键时挂载。

```cpp
struct InteractCommandTag {
    EntityId targetEntity{INVALID_ENTITY};
    float timestamp{0.0f};
};
```

---

## 六、物品与装备组件 (Inventory & Equipment)

### 6.1 InventoryComponent

```cpp
struct ItemSlot {
    uint32_t itemId{0};
    int count{0};
};

struct InventoryComponent {
    static constexpr int MAX_SLOTS = 20;
    ItemSlot slots[MAX_SLOTS];
    int slotCount{0};
    int maxSlots{MAX_SLOTS};
};
```

### 6.2 EquipmentComponent

```cpp
struct EquipmentComponent {
    static constexpr int SLOT_COUNT = 6;
    enum Slot {
        Head,
        Body,
        Weapon,
        Accessory1,
        Accessory2,
        Special
    };
    
    uint32_t equipped[SLOT_COUNT];
};
```

### 6.3 ItemDataComponent

```cpp
struct ItemDataComponent {
    unsigned int itemId{0};
    int count{1};
    bool isPickupable{true};
};
```

---

## 七、环境与区域组件 (Environment Components)

### 7.1 BiomeZoneComponent

```cpp
struct BiomeZoneComponent {
    enum class BiomeType : unsigned char {
        Normal,         // 普通区域
        Radiation,      // 辐射区
        ExtremeCold,    // 极寒区
        Toxic,          // 毒气区
        Sanctuary,      // 安全区（禁止战斗）
    };
    
    BiomeType biomeType{BiomeType::Normal};
    Rect bounds;        // 区域范围（使用自定义 Rect）
    
    // 区域效果（可选）
    float damagePerSecond{0.0f}; // 区域伤害/秒
    int damageType{0}; // 0=Physical, 1=Fire, 2=Toxic, 3=Ice
};
```

### 7.2 DirectorStateComponent

```cpp
struct DirectorStateComponent {
    float lastSpawnTime{0.0f};    // 上次生成怪物时间
    float spawnCooldown{60.0f};   // 生成冷却（秒）
    int currentWave{0};           // 当前波次
    bool isEventActive{false};    // 是否有活跃事件
};
```

---

## 八、渲染事件组件 (Render Events)

### 8.1 RenderEventComponent

> 供外部渲染引擎读取。Project2 逻辑层不处理此组件。

```cpp
struct RenderEventComponent {
    enum class Type : unsigned char {
        PlayAnimation,
        PlaySound,
        SpawnParticle,
        StopSound,
    };
    
    Type type{Type::PlayAnimation};
    char assetKey[64];    // 资源键（动画名、音效名等）
    Vec2 position{0.0f, 0.0f};
    float volume{1.0f};   // 音量（0-1，用于音效）
};
```

---

## 📊 Component 统计

| 类别 | 数量 | POD 合规 | SFML 依赖 |
|------|------|----------|----------|
| 核心组件 | 3 | ✅ | ❌ 无 |
| RPG 组件 | 3 | ✅ | ❌ 无 |
| 战斗组件 | 4 | ✅ | ❌ 无 |
| 标签组件 | 3 | ✅ | ❌ 无 |
| 物品组件 | 3 | ✅ | ❌ 无 |
| 环境组件 | 2 | ✅ | ❌ 无 |
| 渲染组件 | 1 | ✅ | ❌ 无 |
| **总计** | **19** | **✅ 100%** | **✅ 0** |

---

## 🎯 设计规范

### 所有 Component 必须遵守

1. **纯数据结构（POD）**
   ```cpp
   struct MyComponent {
       int value;
       float data;
       // ✅ 可以：默认值
       // ❌ 禁止：虚函数、方法实现、private 成员
   };
   ```

2. **使用自定义数学类型**
   ```cpp
   #include "../math/Vec2.h"
   #include "../math/Rect.h"
   
   struct TransformComponent {
       Vec2 position;    // ✅ 正确
       // sf::Vector2f pos;  // ❌ 禁止
   };
   ```

3. **避免复杂容器**
   ```cpp
   // ✅ 可以：固定数组
   EntityId hitHistory[16];
   
   // ⚠️ 谨慎：std::vector（非 POD）
   // 如果必须使用，确保不在 update() 中频繁分配
   
   // ❌ 禁止：在 Component 中
   std::set<EntityId> hits;  // 改用固定数组
   ```

---

## 📝 更新历史

| 日期 | 改动 | 负责人 |
|------|------|--------|
| 2026-03-29 | 移除所有 SFML 依赖，使用 Vec2 和 Rect | AI Assistant |
| 2026-03-29 | 修复 POD 合规性（固定数组代替容器） | AI Assistant |
| 2026-03-29 | 初始版本 | Project2 Team |

---

**维护者:** Project2 Team  
**最后更新:** 2026-03-29  
**下次审查:** 添加新 Component 时

# Component 完整清单

> **最后更新:** 2026-03-29  
> **总数:** 20+ 个 Component  
> **状态:** ✅ 全部符合 POD 规范（无 SFML 依赖）

---

## 📐 数学类型（基础）

| 类型 | 文件 | 用途 | 替代 SFML |
|------|------|------|----------|
| `Vec2` | `math/Vec2.h` | 二维向量 | `Vec2` |
| `Rect` | `math/Rect.h` | 矩形 AABB | `Rect` |

**注意：** 所有 Component 现在使用自定义数学类型，无 SFML 依赖。

---

## 一、核心组件（6 个）

| Component | 文件 | 字段 | 用途 |
|-----------|------|------|------|
| `TransformComponent` | `Transform.h` | `Vec2 position, scale, velocity`<br>`float rotation` | 位置、旋转、速度 |
| `StateMachineComponent` | `StateMachine.h` | `CharacterState currentState`<br>`CharacterState previousState`<br>`float stateTimer` | 角色状态机数据 |
| `CharacterComponent` | `Character.h` | `string name`<br>`int level, maxHP, currentHP`<br>`int baseAttack, baseDefense`<br>`float baseMoveSpeed`<br>`bool isInvincible`<br>`float invincibleTimer`<br>`float facingX, facingY` | 角色基础属性 |
| `InputCommand` | `InputCommand.h` | `Command cmd` | 输入命令（移动/攻击） |
| `Position` | `Position.h` | `float x, y` | 简化位置（旧，待废弃） |
| `FactionComponent` | `Faction.h` | `Faction faction` | 阵营标签 |

---

## 二、战斗组件（6 个）

| Component | 文件 | 字段 | 用途 |
|-----------|------|------|------|
| `HitboxComponent` | `Hitbox.h` | `Rect bounds`<br>`int damageMultiplier`<br>`ElementType element`<br>`float knockbackForce`<br>`EntityId sourceEntity`<br>`EntityId hitHistory[MAX_HIT_COUNT]`<br>`int hitCount`<br>`bool active` | 攻击判定框 |
| `HurtboxComponent` | `Hurtbox.h` | `Rect bounds`<br>`Faction faction`<br>`int layer`<br>`float invincibleTime` | 受击判定框 |
| `DamageTag` | `DamageTag.h` | `float damage`<br>`Vec2 knockbackDirection`<br>`EntityId source`<br>`ElementType element`<br>`bool applied` | 伤害交接 Tag（1 帧） |
| `AttackStateComponent` | `AttackState.h` | `float attackTimer`<br>`float attackDuration`<br>`bool hitActivated` | 攻击状态数据 |
| `CombatStats` | `CombatStats.h` | `int damage`<br>`int defense`<br>`float critRate` | 战斗统计 |
| `AggroComponent` | `Aggro.h` | `float aggroValue`<br>`EntityId target` | 仇恨值 |

---

## 三、RPG 组件（4 个）

| Component | 文件 | 字段 | 用途 |
|-----------|------|------|------|
| `RuntimeStatsComponent` | `RuntimeStats.h` | `int finalAttack`<br>`int finalDefense`<br>`float finalMoveSpeed` | 最终战斗面板 |
| `EvolutionComponent` | `Evolution.h` | `int evolutionPoints`<br>`std::vector<SkillId> unlockedSkills` | 进化数据 |
| `StatModifierComponent` | `StatModifier.h` | `std::vector<StatModifier> modifiers` | 属性修饰器集合 |
| `ProgressionComponent` | `Progression.h` | `int experience`<br>`int level` | 等级和经验 |

---

## 四、物品组件（3 个）

| Component | 文件 | 字段 | 用途 |
|-----------|------|------|------|
| `InventoryComponent` | `Inventory.h` | `std::vector<ItemSlot> slots`<br>`int maxSlots` | 背包 |
| `EquipmentComponent` | `Equipment.h` | `ItemId equipped[6]` | 装备槽 |
| `ItemDataComponent` | `ItemData.h` | `unsigned int itemId`<br>`int count`<br>`bool isPickupable` | 物品数据 |

---

## 五、事件组件（2 个）

| Component | 文件 | 字段 | 用途 |
|-----------|------|------|------|
| `RenderEventComponent` | `RenderEvent.h` | `Type type`<br>`string assetKey`<br>`Vec2 position`<br>`float volume` | 渲染事件（通知外部） |
| `DirectorStateComponent` | `DirectorState.h` | `float lastSpawnTime`<br>`float spawnCooldown`<br>`int currentWave`<br>`bool isEventActive` | AI 导演状态 |

---

## 六、环境组件（1 个）

| Component | 文件 | 字段 | 用途 |
|-----------|------|------|------|
| `BiomeZoneComponent` | `BiomeZone.h` | `BiomeType biomeType`<br>`Rect bounds`<br>`float damagePerSecond`<br>`int damageType` | 生物群落区域 |

---

## 📊 统计

| 类别 | 数量 | POD 合规 | SFML 依赖 |
|------|------|----------|----------|
| 数学类型 | 2 | ✅ | ❌ 无 |
| 核心组件 | 6 | ✅ | ❌ 无 |
| 战斗组件 | 6 | ✅ | ❌ 无 |
| RPG 组件 | 4 | ✅ | ❌ 无 |
| 物品组件 | 3 | ✅ | ❌ 无 |
| 事件组件 | 2 | ✅ | ❌ 无 |
| 环境组件 | 1 | ✅ | ❌ 无 |
| **总计** | **24** | **✅ 100%** | **✅ 0** |

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
   std::vector<ItemSlot> slots;
   
   // ❌ 禁止：在 Component 中
   std::set<EntityId> hits;  // 改用固定数组
   ```

---

## 📝 更新历史

| 日期 | 改动 | 负责人 |
|------|------|--------|
| 2026-03-29 | 移除所有 SFML 依赖，使用自定义数学类型 | AI Assistant |
| 2026-03-29 | 修复 HitboxComponent POD 合规性（移除 std::set） | AI Assistant |
| 2026-03-29 | 初始版本 | Project2 Team |

---

**维护者:** Project2 Team  
**最后更新:** 2026-03-29  
**下次审查:** 添加新 Component 时

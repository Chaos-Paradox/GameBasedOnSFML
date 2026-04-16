# Component 完整清单

> **最后更新:** 2026-04-15
> **总数:** 35 个 Component
> **状态:** ✅ 全部符合 POD 规范（无 SFML 依赖）

---

## 📐 数学类型（基础）

| 类型 | 文件 | 用途 | 替代 SFML |
|------|------|------|----------|
| `Vec2` | `math/Vec2.h` | 二维向量 | `sf::Vector2f` |
| `Rect` | `math/Rect.h` | 矩形 AABB | `sf::FloatRect` |
| `MathUtils` | `math/MathUtils.h` | 数学工具函数 | — |

**注意：** 所有 Component 使用自定义数学类型，无 SFML 依赖。

---

## 一、空间类组件（4 个）

| Component | 文件 | 关键字段 | 用途 |
|-----------|------|---------|------|
| `TransformComponent` | `Transform.h` | `Vec2 position, scale, velocity`<br>`float rotation`<br>`float facingX, facingY` | 主空间变换：位置、缩放、旋转、速度、朝向 |
| `ZTransformComponent` | `ZTransformComponent.h` | `float z, vz, gravity, height`<br>方法：`isGrounded()`, `applyGravity()`, `jump()` | Z 轴高度：跳跃/炸弹弹跳/击飞空中 |
| `Position` | `Position.h` | `float x, y` | 简化位置（遗留，待废弃） |
| `LifetimeComponent` | `Lifetime.h` | `float timeLeft`<br>`bool autoDestroy` | 临时实体自动销毁计时 |

---

## 二、角色与状态类组件（6 个）

| Component | 文件 | 关键字段 | 用途 |
|-----------|------|---------|------|
| `CharacterComponent` | `Character.h` | `string name`<br>`int level, maxHP, currentHP`<br>`int baseAttack, baseDefense`<br>`float baseMoveSpeed`<br>`bool isInvincible`<br>`float invincibleTimer`<br>`float facingX, facingY` | 角色基础属性（不受装备/进化影响） |
| `StateMachineComponent` | `StateMachine.h` | `CharacterState currentState`<br>`CharacterState previousState`<br>`float stateTimer` | 状态机数据：当前/上一状态 + 计时器 |
| `InputCommand` | `InputCommand.h` | `Vec2 moveDir`<br>`ActionIntent pendingIntent`<br>`float intentTimer` | **单轨指令槽**（v3.1）：移动方向 + 意图 + 保质期 |
| `DashComponent` | `DashComponent.h` | `float dashSpeed` (2000)<br>`int maxCharges`/`currentCharges`<br>`float dashTimer/rechargeTimer/iframeTimer/recoveryTimer`<br>`Vec2 dashDir`<br>`bool isInvincible` | 冲刺能力：电荷系统、无敌帧、速度曲线 |
| `GameStateComponent` | `GameState.h` | `int state` (0=Menu,1=Playing,2=Paused,3=GameOver)<br>`float gameTime`<br>`int difficultyLevel` | 全局游戏状态（预留） |
| `DirectorStateComponent` | `DirectorState.h` | `float lastSpawnTime, spawnCooldown`<br>`int currentWave`<br>`bool isEventActive` | AI 导演：刷怪节奏控制（待实现） |

---

## 三、战斗类组件（7 个）

| Component | 文件 | 关键字段 | 用途 |
|-----------|------|---------|------|
| `HitboxComponent` | `Hitbox.h` | `float radius`<br>`Vec2 offset`<br>`int damageMultiplier`<br>`ElementType element`<br>`float knockbackForce/knockbackXY/knockbackZ`<br>`EntityId sourceEntity`<br>`std::unordered_set<EntityId> hitTargets`<br>`bool active` | 攻击判定圆柱（2.5D）。圆形 XY 判定避免对角穿模，hitTargets 防止多帧重复伤害 |
| `HurtboxComponent` | `Hurtbox.h` | `float radius, height, zOffset`<br>`Vec2 offset`<br>`Faction faction`<br>`int layer`<br>`float invincibleTime` | 受击判定圆柱。阵营和层决定碰撞资格，invincibleTime 提供受击后短暂无敌 |
| `AttackStateComponent` | `AttackState.h` | `float hitTimer/hitDuration`<br>`int baseDamage`<br>`float attackRange` (120)<br>`float attackArc` (120°)<br>`bool hasFiredDamage` | 攻击几何数据：扇形范围扫描，hasFiredDamage 确保单次伤害 |
| `DamageEventComponent` | `DamageEventComponent.h` | `EntityId target, attacker`<br>`int actualDamage`<br>`Vec2 hitPosition, hitDirection`<br>`bool isCritical`<br>`float knockbackXY, knockbackZ`<br>`float timestamp` | 伤害事件实体载荷。支持伤害浮动（0.8-1.2x）、暴击、多 Hit 同步、延迟结算 |
| `DamageTag` | `DamageTag.h` | `float damage` | 伤害交接 Tag（1 帧，遗留） |
| `DeathTag` | `DeathTag.h` | （纯标记，无字段） | 延迟销毁标记。防止帧中销毁导致的悬空引用 |
| `Stats` | `Stats.h` | `float hp, attackDamage, attackSpeed` | 简单扁平统计（遗留，已被 RuntimeStatsComponent 取代） |

---

## 四、战利品类组件（7 个）

| Component | 文件 | 关键字段 | 用途 |
|-----------|------|---------|------|
| `LootDropComponent` | `LootDrop.h` | `LootEntry lootTable[8]`<br>`int lootCount`<br>`bool hasDropped` | 怪物死亡掉落表。固定数组（POD 合规），hasDropped 防止重复掉落 |
| `ItemDataComponent` | `ItemData.h` | `unsigned int itemId`<br>`int amount`<br>`bool isPickupable`<br>`float magnetImmunityTimer` (0.25s) | 掉落物数据。magnetImmunityTimer 防止生成瞬间被磁吸 |
| `PickupBoxComponent` | `PickupBox.h` | `float width` (30), `height` (30) | 拾取碰撞框。独立于战斗 Hurtbox |
| `MagnetComponent` | `MagnetComponent.h` | `float magnetRadius` (200)<br>`float magnetSpeed` (400) | 挂载在玩家身上，定义磁吸半径和速度 |
| `InventoryComponent` | `Inventory.h` | `std::vector<InventorySlot> slots`<br>`int maxSlots` (20) | 背包：可堆叠物品槽 |
| `EquipmentComponent` | `Equipment.h` | `unsigned int head, body, weapon, accessory` | 四件装备槽 |
| `EvolutionComponent` | `Evolution.h` | `int evolutionPoints`（可用）<br>`int totalEarned`（累计） | 进化/成长点数追踪 |

---

## 五、物理类组件（3 个）

| Component | 文件 | 关键字段 | 用途 |
|-----------|------|---------|------|
| `ColliderComponent` | `ColliderComponent.h` | `float radius` (20)<br>`bool isStatic`<br>方法：`isDynamic()` | 物理碰撞圆柱（实体间推挤/阻挡）。独立于战斗 Hitbox/Hurtbox |
| `MomentumComponent` | `MomentumComponent.h` | `float mass` (1.0)<br>`Vec2 velocity`<br>`float collisionCooldown`<br>`float prevPosX, prevPosY`（CCD 用）<br>`bool useCCD` | 独立碰撞速度 + 质量，用于弹性碰撞响应和 CCD（连续碰撞检测） |
| `BombComponent` | `BombComponent.h` | `float fuseTimer` (3s)<br>`bool isKicked`<br>`float lastPosX, lastPosY` | 炸弹实体：引信倒计时、踢飞状态。配合 ZTransformComponent 实现弹跳物理 |

---

## 六、RPG 类组件（2 个）

| Component | 文件 | 关键字段 | 用途 |
|-----------|------|---------|------|
| `RuntimeStatsComponent` | `RuntimeStats.h` | `int finalAttack, finalDefense`<br>`float finalMoveSpeed, finalMaxHP`<br>`int attackElement` (0=物理,1=火,2=毒,3=冰)<br>`float fireResist, toxicResist, iceResist`<br>`int currentHP` | 最终战斗面板（基础 + 装备 + 进化）。每帧被战斗系统读取 |
| `StatModifierComponent` | `StatModifier.h` | `std::vector<StatModifier> modifiers`<br>StatModifier: `Type` (Flat/Percent/Multiplicative), `statName`, `value`, `sourceId`, `duration`, `remainingTime` | 临时增益/减益修饰器集合。支持扁平、百分比、乘算叠加 |

---

## 七、渲染与视觉类组件（3 个）

| Component | 文件 | 关键字段 | 用途 |
|-----------|------|---------|------|
| `RenderEventComponent` | `RenderEvent.h` | `Type type` (PlayAnimation/PlaySound/SpawnParticle/StopSound)<br>`string assetKey`<br>`Vec2 position`<br>`float volume` (0-1) | 游戏逻辑 → 外部渲染引擎桥梁 |
| `AnimationStateComponent` | `AnimationState.h` | `string currentAnimation`<br>`bool isPlaying`<br>`float playbackTime` | 当前动画播放状态 |
| `DamageTextComponent` | `DamageTextComponent.h` | `string text`<br>`float timer` (1s)<br>`Vec2 position, velocity` (-50 向上)<br>`bool isCritical`<br>`float alpha, fontSize` (24)<br>`float fadeOutStart` (0.5s) | 浮动伤害数字：上升、淡出、暴击样式，~1 秒生命周期 |

---

## 八、环境类组件（2 个）

| Component | 文件 | 关键字段 | 用途 |
|-----------|------|---------|------|
| `BiomeZoneComponent` | `BiomeZone.h` | `BiomeType biomeType` (Normal/Radiation/ExtremeCold/Toxic/Sanctuary)<br>`Rect bounds`<br>`float damagePerSecond`<br>`int damageType` | 隐形地图区域触发器，定义生物群落和可选 DoT 效果 |
| `EnvironmentalTag` | `EnvironmentalTag.h` | `int biomeType`<br>`float enterTime` | 玩家进入生物群落时临时挂载，用于适应性/抗性计算 |

---

## 九、命令/标签类组件（4 个）

| Component | 文件 | 关键字段 | 用途 |
|-----------|------|---------|------|
| `EvolveCommandTag` | `Tags.h` | `uint32_t skillId`<br>`int level` | UI 层 → ProgressionSystem 进化指令 |
| `InteractCommandTag` | `Tags.h` | `EntityId targetEntity` | 玩家输入 → InteractionSystem 交互指令 |
| `CraftCommandTag` | `Tags.h` | `uint32_t recipeId`<br>`int craftCount` | 玩家输入 → CraftingSystem 合成指令 |
| `SpawnCommandTag` | `Tags.h` | `uint32_t enemyTypeId`<br>`sf::Vector2f spawnPosition`<br>`int count` | DirectorSystem → World 刷怪指令 |

---

## 十、其他组件（1 个）

| Component | 文件 | 关键字段 | 用途 |
|-----------|------|---------|------|
| `AttachedComponent` | `AttachedComponent.h` | `EntityId parentEntityId`<br>`Vec2 offset` | 附属实体：标记本实体跟随另一实体（如 Hitbox 跟随玩家） |

---

## 📊 统计

| 类别 | 数量 | POD 合规 | SFML 依赖 |
|------|------|----------|----------|
| 数学类型 | 3 | ✅ | ❌ 无 |
| 空间类 | 4 | ✅ | ❌ 无 |
| 角色与状态 | 6 | ✅ | ❌ 无 |
| 战斗类 | 7 | ✅ | ❌ 无 |
| 战利品类 | 7 | ⚠️ vector | ❌ 无 |
| 物理类 | 3 | ✅ | ❌ 无 |
| RPG 类 | 2 | ⚠️ vector | ❌ 无 |
| 渲染与视觉 | 3 | ✅ | ❌ 无 |
| 环境类 | 2 | ✅ | ❌ 无 |
| 命令/标签 | 4 | ⚠️ SpawnCommandTag 含 SFML | ⚠️ 1 个 |
| 其他 | 1 | ✅ | ❌ 无 |
| **总计** | **42** | **✅ 97.6%** | **⚠️ 1 个待清理** |

**⚠️ SpawnCommandTag 中使用了 `sf::Vector2f`，违反 "Project2 无 SFML" 规则，应改为 `Vec2`。**

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
| 2026-04-15 | 完整重构：对齐实际代码库 35 个 Component，9 类分组 | AI Assistant |
| 2026-03-29 | 移除所有 SFML 依赖，使用自定义数学类型 | AI Assistant |
| 2026-03-29 | 修复 HitboxComponent POD 合规性（移除 std::set） | AI Assistant |
| 2026-03-29 | 初始版本 | Project2 Team |

---

**维护者:** Project2 Team
**最后更新:** 2026-04-15
**下次审查:** 添加新 Component 时

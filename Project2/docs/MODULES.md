# MODULES.md - Project2 模块职责清单

> **最后更新:** 2026-04-15
> **总数:** 9 个模块域, 35 个 Components, 21 个 Systems

---

## 模块概览

| 模块 | 路径 | 职责 | 文件数 |
|------|------|------|--------|
| Core 核心 | `include/core/` | ECS 基础设施、实体管理、世界上下文 | 8 |
| Math 数学 | `include/math/` | 自定义 2D 数学类型和工具函数 | 3 |
| Components 组件 | `include/components/` | 纯数据 POD 结构，共 35 个 | 35 |
| Systems 系统 | `include/systems/` | 无状态处理逻辑，共 21 个 | 21 |
| States 状态 | `include/states/` | 状态机接口 + 5 个具体状态 | 6 |
| Factories 工厂 | `include/factories/` | 实体创建工厂 | 2 |
| Utils 工具 | `include/utils/` | 碰撞类型、日志、遗留数学工具 | 4 |
| Tests 测试 | `tests/` | 可视化沙盒 + 单元测试 | - |
| Data 数据 | `data/` | JSON 预制体文件 | - |

---

## 一、Core 模块（ECS 基础设施）

| 文件 | 类/结构 | 职责 |
|------|---------|------|
| `ECS.h` | `ECS` | 实体创建（Free List ID 回收）和销毁。维护 `created_order` 活动实体列表和 `freeList` 回收队列 |
| `Entity.h` | `Entity`（typedef） | 实体 ID 类型别名（`uint32_t`），定义 `INVALID_ENTITY = 0` 哨兵值 |
| `Component.h` | `ComponentStore<T>` | 泛型组件容器，使用 `std::unordered_map<Entity, T>` 存储，提供 add/has/get/remove/entityList |
| `GameWorld.h` | `GameWorld` | 中央枢纽：持有所有 21 个 ComponentStore、ECS 实例、GameJuice、InputManager、玩家引用、围栏实体列表 |
| `EntityFactory.h` | `EntityFactory` | JSON 驱动实体工厂，从 `data/prefabs/` 加载配置，提供 spawnPlayer/spawnDummy/spawnThrowableBomb/spawnExplosion |
| `InputManager.h` | `InputManager` | 双玩家独立输入管理器，支持 JSON 持久化键位配置，GameAction→EngineKey 映射，冲突检测 |
| `GameJuice.h` | `GameJuice` | 全局游戏感觉状态：timeScale（hit-stop/slow-mo）、hitStopTimer、shakeTimer、shakeIntensity |
| `GameConfig.h` | — | 空占位文件（待实现） |

---

## 二、Math 模块（自定义数学库）

| 文件 | 类型 | 职责 |
|------|------|------|
| `Vec2.h` | `Vec2` | 2D 向量结构体。运算符重载（+、-、*、/、+=、-=、*=、==、!=），方法（length、lengthSquared、normalized、dot），静态常量（Zero、One、Up、Down、Left、Right） |
| `Rect.h` | `Rect` | AABB 矩形结构体。边界查询（left/right/top/bottom、centerX/Y），碰撞检测（overlaps、contains），变换（translate、scale） |
| `MathUtils.h` | `Math::` 命名空间 | 工具函数：distance、distanceSquared、angle、inSector（锥形检测）、lerp（标量/向量）、clamp、rectOverlap、rectContains、circleRectOverlap。常量：PI、DEG2RAD、RAD2DEG |

---

## 三、Systems 模块（21 个系统）

### 3.1 输入与状态系统

| 系统 | 帧阶段 | 读取 | 写入 | 职责 |
|------|--------|------|------|------|
| `StateMachineSystem` | Phase 1 | InputCommand, StateMachineComponent, AttackStateComponent, DamageEventComponent | StateMachineComponent, InputCommand, 创建 AttackStateComponent | 核心状态机：时间静止魔法、意图消费、状态切换（Enter/Update/Exit）、攻击取消窗口 |
| `DashSystem` | Phase 1 | DashComponent, StateMachineComponent, TransformComponent, InputCommand | DashComponent, StateMachineComponent, TransformComponent.velocity | 冲刺管理：电荷系统（最多 2 电荷，自动回复）、无敌帧、速度曲线（恒速→指数摩擦衰减）、Recovery 状态过渡 |
| `InputSystem` | — | InputCommand | InputCommand.cmd | 遗留输入系统，写入原始 Command 枚举值（已被 InputManager + StateMachineSystem 取代） |

### 3.2 移动系统

| 系统 | 帧阶段 | 读取 | 写入 | 职责 |
|------|--------|------|------|------|
| `LocomotionSystem` | Phase 2 | StateMachineComponent, TransformComponent, CharacterComponent, InputCommand | TransformComponent.facingX/Y, TransformComponent.velocity | 移动速度计算：输入方向归一化 × baseMoveSpeed，跳过 Dash/Attack/Hurt/Dead/KnockedAirborne/Recovery 状态 |
| `MovementSystem` | Phase 3 | TransformComponent, StateMachineComponent, ZTransformComponent, BombComponent | TransformComponent.velocity（摩擦/限速）, TransformComponent.position | 位置积分（delta-time 移动）、摩擦力（地面强/空气弱 0.99）、1800px/s 速度上限防空穿 |
| `MagnetSystem` | Phase 2 | MagnetComponent, TransformComponent（玩家）, ItemDataComponent, TransformComponent（掉落物） | ItemDataComponent.magnetImmunityTimer, TransformComponent.velocity（掉落物） | 磁吸系统：玩家在 magnetRadius 内时，掉落物以 magnetSpeed 飞向玩家；超出范围时速度阻尼 |

### 3.3 物理与碰撞系统

| 系统 | 帧阶段 | 读取 | 写入 | 职责 |
|------|--------|------|------|------|
| `PhysicalCollisionSystem` | Phase 3 | ColliderComponent, TransformComponent, MomentumComponent, ZTransformComponent, BombComponent, StateMachineComponent | TransformComponent.position（重叠解析）, TransformComponent.velocity（弹性碰撞响应）, MomentumComponent | 物理碰撞：质量权重重叠解析 + 弹性碰撞响应 + CCD（连续碰撞检测）防空穿 + Z 轴过滤（空中实体跳过地面碰撞） |
| `AttachmentSystem` | Phase 3 | AttachedComponent, TransformComponent, ZTransformComponent | TransformComponent.position, ZTransformComponent.z | 附属实体同步：将附属实体（如 Hitbox）每帧同步到宿主的位置 + Z 偏移 |

### 3.4 炸弹系统

| 系统 | 帧阶段 | 读取 | 写入 | 职责 |
|------|--------|------|------|------|
| `BombSystem` | Phase 2 | BombComponent, TransformComponent, ZTransformComponent, MomentumComponent, StateMachineComponent, DashComponent, ColliderComponent, DeathTag | BombComponent（fuseTimer/isKicked/lastPos）, TransformComponent.velocity（玩家和炸弹）, MomentumComponent, ZTransformComponent, 创建爆炸 Hitbox | 炸弹生命周期管理：引信倒计时、弹跳物理、Dash 踢飞 CCD 碰撞检测（弹性碰撞响应，基于质量）、爆炸 AOE 生成 |

### 3.5 战斗管线系统

| 系统 | 帧阶段 | 读取 | 写入 | 职责 |
|------|--------|------|------|------|
| `AttackSystem` | Phase 4 | StateMachineComponent, AttackStateComponent, TransformComponent, ZTransformComponent, HurtboxComponent | TransformComponent.velocity（攻击中归零）, 创建 DamageEventComponent 实体 | 攻击扇形扫描：检测敌人 Hurtbox 是否在范围内、角度阈值内、Z 轴重叠。创建伤害事件（随机伤害 0.8x-1.2x + 暴击概率），单次攻击单次伤害 |
| `CollisionSystem` | Phase 4 | HitboxComponent, HurtboxComponent, TransformComponent, ZTransformComponent | HitboxComponent.hitTargets, 创建 DamageEventComponent 实体 | Hitbox-vs-Hurtbox 2.5D 圆柱碰撞检测（圆形 XY + Z 轴区间重叠）。通过 hitTargets 防止多帧重复伤害。创建随机化伤害和击飞值的伤害事件 |
| `CombatSystem` | — | Position, AttackState, Stats | Stats.hp | 遗留/简化战斗系统，基于扇形的攻击检测直接修改 HP（已被 AttackSystem + CollisionSystem + DamageSystem 管线取代） |
| `DamageSystem` | Phase 4 | DamageEventComponent, CharacterComponent, StateMachineComponent, DashComponent | CharacterComponent.currentHP, StateMachineComponent.currentState（设为 KnockedAirborne）, TransformComponent.velocity（击飞）, ZTransformComponent, 创建 DamageTextComponent, 设置 GameJuice, 添加 DeathTag | 伤害事件处理：应用伤害到 HP、检查冲刺无敌（闪避）、生成伤害数字实体、应用击飞速度、触发 GameJuice（hit-stop + 屏幕震动）、HP 归零时标记死亡 |
| `DamageTextSpawnerSystem` | — | DamageEventComponent | 创建 DamageTextComponent 实体 | 从伤害事件生成伤害文字实体（注意：DamageSystem 已直接创建伤害文字，此系统可能冗余） |
| `DamageTextRenderSystem` | Phase 7 | DamageTextComponent, TransformComponent | DamageTextComponent.timer/position/alpha, 销毁到期实体 | 伤害数字渲染（需要 SFML）：浮动动画（速度驱动）、淡出、暴击样式（粗体红色）、生命周期到期销毁。ENABLE_SFML 未定义时为无操作存根 |

### 3.6 战后处理系统

| 系统 | 帧阶段 | 读取 | 写入 | 职责 |
|------|--------|------|------|------|
| `LootSpawnSystem` | Phase 5 | DeathTagComponent, LootDropComponent, TransformComponent | LootDropComponent.hasDropped, 创建掉落物实体（Transform + ItemData + PickupBox） | 死亡掉落生成：读取实体死亡标记和掉落表，随机掉落判定（概率 + 数量抖动 + 位置抖动 + 爆米花抛射初速度） |
| `DeathSystem` | Phase 5 | DeathTagComponent, StateMachineComponent, EvolutionComponent | StateMachineComponent.currentState = Dead | 死亡状态设置：拥有 DeathTag 的实体设为 Dead 状态。玩家（有 EvolutionComponent）免疫死亡——移除 DeathTag 而非进入清理 |
| `PickupSystem` | Phase 5 | EvolutionComponent, TransformComponent（玩家）, ItemDataComponent, TransformComponent（掉落物）, PickupBoxComponent | EvolutionComponent.evolutionPoints/totalEarned, 销毁已拾取掉落物实体 | 拾取判定：玩家与掉落物 AABB 重叠检测。拾取时增加进化点数并销毁掉落物实体 |

### 3.7 清理与渲染系统

| 系统 | 帧阶段 | 读取 | 写入 | 职责 |
|------|--------|------|------|------|
| `CleanupSystem` | Phase 6 | DeathTagComponent, LifetimeComponent | 移除所有组件存储中的实体，调用 ECS.destroy() | 帧末延迟销毁：收集所有 DeathTag 实体或 Lifetime 到期实体，剥离所有组件（"孟婆汤"防止实体复用污染），销毁实体 ID |
| `RenderSystem` | Phase 7 | 几乎所有组件（角色、变换、状态、冲刺、Z 变换、 Hurtbox、Hitbox、攻击状态、物品、炸弹、碰撞体、GameJuice） | GameJuice.shakeTimer/shakeIntensity（消费震动状态） | 2.5D 伪 3D 渲染（需要 SFML）：Y 排序（根据 Z 高度调整）、网格背景、围栏墙壁、实体阴影（大小/透明度基于 Z 高度）、实体（按状态着色：玩家/敌人/受伤/死亡/冲刺）、Hitbox（黄色圆）、攻击扇形（红色）、掉落物（黄色圆）、炸弹（黑色圆 + 闪烁引信）、伤害文字。ENABLE_SFML 未定义时为无操作存根 |
| `DebugSystem` | Phase 7 | 所有组件存储（通过 GameWorld） | 无（仅控制台输出） | 格式化打印实体和掉落物列表到控制台，用于调试。显示实体 ID、位置、HP、状态、物品数据、进化点 |

---

## 四、Components 模块（35 个组件）

### 4.1 空间类（4 个）

| 组件 | 文件 | 关键字段 | 所属系统 |
|------|------|---------|---------|
| `TransformComponent` | `Transform.h` | position, scale, rotation, velocity, facingX, facingY | LocomotionSystem, MovementSystem, PhysicalCollisionSystem, 几乎所有系统 |
| `ZTransformComponent` | `ZTransformComponent.h` | z, vz, gravity, height | CollisionSystem, MovementSystem, PhysicalCollisionSystem, BombSystem, AttackSystem |
| `Position` | `Position.h` | x, y | CombatSystem（遗留） |
| `LifetimeComponent` | `Lifetime.h` | timeLeft, autoDestroy | CleanupSystem |

### 4.2 角色与状态类（6 个）

| 组件 | 文件 | 关键字段 | 所属系统 |
|------|------|---------|---------|
| `CharacterComponent` | `Character.h` | name, level, maxHP/currentHP, baseAttack/baseDefense, baseMoveSpeed, isInvincible/invincibleTimer, facingX/facingY | LocomotionSystem, DamageSystem, DebugSystem |
| `StateMachineComponent` | `StateMachine.h` | currentState, previousState, stateTimer | StateMachineSystem, DashSystem, LocomotionSystem, MovementSystem, DeathSystem, AttackSystem |
| `InputCommand` | `InputCommand.h` | moveDir, pendingIntent, intentTimer | StateMachineSystem, DashSystem, LocomotionSystem, InputSystem |
| `DashComponent` | `DashComponent.h` | dashSpeed, maxCharges/currentCharges, dashTimer/rechargeTimer/iframeTimer/recoveryTimer, dashDir, isInvincible | DashSystem, DamageSystem, BombSystem |
| `GameStateComponent` | `GameState.h` | state, gameTime, difficultyLevel | （预留，尚未有系统使用） |
| `DirectorStateComponent` | `DirectorState.h` | lastSpawnTime, spawnCooldown, currentWave, isEventActive | （AI 导演，待实现） |

### 4.3 战斗类（7 个）

| 组件 | 文件 | 关键字段 | 所属系统 |
|------|------|---------|---------|
| `HitboxComponent` | `Hitbox.h` | radius, offset, damageMultiplier, element, knockbackForce/knockbackXY/knockbackZ, sourceEntity, hitTargets, active | CollisionSystem, AttachmentSystem, RenderSystem |
| `HurtboxComponent` | `Hurtbox.h` | radius, height, zOffset, offset, faction, layer, invincibleTime | AttackSystem, CollisionSystem |
| `AttackStateComponent` | `AttackState.h` | hitTimer/hitDuration, baseDamage, attackRange, attackArc, hasFiredDamage | AttackSystem, StateMachineSystem |
| `DamageEventComponent` | `DamageEventComponent.h` | target, actualDamage, hitPosition, isCritical, hitDirection, knockbackXY/knockbackZ, attacker, timestamp | DamageSystem, DamageTextSpawnerSystem, StateMachineSystem |
| `DamageTag` | `DamageTag.h` | damage | DamageSystem（遗留） |
| `DeathTag` | `DeathTag.h` | （纯标记） | DeathSystem, LootSpawnSystem, CleanupSystem, BombSystem |
| `Stats` | `Stats.h` | hp, attackDamage, attackSpeed | CombatSystem（遗留） |

### 4.4 战利品类（7 个）

| 组件 | 文件 | 关键字段 | 所属系统 |
|------|------|---------|---------|
| `LootDropComponent` | `LootDrop.h` | lootTable[8], lootCount, hasDropped | LootSpawnSystem |
| `ItemDataComponent` | `ItemData.h` | itemId, amount, isPickupable, magnetImmunityTimer | MagnetSystem, PickupSystem, RenderSystem |
| `PickupBoxComponent` | `PickupBox.h` | width, height | PickupSystem |
| `MagnetComponent` | `MagnetComponent.h` | magnetRadius, magnetSpeed | MagnetSystem |
| `InventoryComponent` | `Inventory.h` | slots（vector）, maxSlots | （背包系统，待实现） |
| `EquipmentComponent` | `Equipment.h` | head, body, weapon, accessory | （装备系统，待实现） |
| `EvolutionComponent` | `Evolution.h` | evolutionPoints, totalEarned | PickupSystem, DeathSystem, DebugSystem |

### 4.5 物理类（3 个）

| 组件 | 文件 | 关键字段 | 所属系统 |
|------|------|---------|---------|
| `ColliderComponent` | `ColliderComponent.h` | radius, isStatic | PhysicalCollisionSystem, BombSystem, RenderSystem |
| `MomentumComponent` | `MomentumComponent.h` | mass, velocity, collisionCooldown, prevPosX/Y, useCCD | PhysicalCollisionSystem, BombSystem |
| `BombComponent` | `BombComponent.h` | fuseTimer, isKicked, lastPosX/Y | BombSystem, MovementSystem, PhysicalCollisionSystem |

### 4.6 RPG 类（2 个）

| 组件 | 文件 | 关键字段 | 所属系统 |
|------|------|---------|---------|
| `RuntimeStatsComponent` | `RuntimeStats.h` | finalAttack/finalDefense/finalMoveSpeed/finalMaxHP, attackElement, resistances, currentHP | （属性计算系统，待实现） |
| `StatModifierComponent` | `StatModifier.h` | modifiers（vector of StatModifier） | （修饰器系统，待实现） |

### 4.7 渲染与视觉类（3 个）

| 组件 | 文件 | 关键字段 | 所属系统 |
|------|------|---------|---------|
| `RenderEventComponent` | `RenderEvent.h` | type, assetKey, position, volume | （外部渲染层读取） |
| `AnimationStateComponent` | `AnimationState.h` | currentAnimation, isPlaying, playbackTime | （动画系统，待实现） |
| `DamageTextComponent` | `DamageTextComponent.h` | text, timer, position, velocity, isCritical, alpha, fontSize, fadeOutStart | DamageTextRenderSystem, DamageTextSpawnerSystem, CleanupSystem |

### 4.8 环境类（2 个）

| 组件 | 文件 | 关键字段 | 所属系统 |
|------|------|---------|---------|
| `BiomeZoneComponent` | `BiomeZone.h` | biomeType, bounds, damagePerSecond, damageType | （环境系统，待实现） |
| `EnvironmentalTag` | `EnvironmentalTag.h` | biomeType, enterTime | （环境系统，待实现） |

### 4.9 命令/标签类（4 个）

| 组件 | 文件 | 关键字段 | 所属系统 |
|------|------|---------|---------|
| `EvolveCommandTag` | `Tags.h` | skillId, level | UI → ProgressionSystem |
| `InteractCommandTag` | `Tags.h` | targetEntity | Player → InteractionSystem |
| `CraftCommandTag` | `Tags.h` | recipeId, craftCount | Player → CraftingSystem |
| `SpawnCommandTag` | `Tags.h` | enemyTypeId, spawnPosition, count | DirectorSystem → World |

### 4.10 其他（1 个）

| 组件 | 文件 | 关键字段 | 所属系统 |
|------|------|---------|---------|
| `AttachedComponent` | `AttachedComponent.h` | parentEntityId, offset | AttachmentSystem |

---

## 五、States 模块（6 个状态）

| 状态 | 文件 | 职责 |
|------|------|------|
| `IState` | `IState.h` | 状态接口：Enter(entity)、Update(entity, dt)、Exit(entity)、GetName()、GetType() |
| `IdleState` | `IdleState.h` | 待命状态：等待输入或意图触发 |
| `MoveState` | `MoveState.h` | 移动状态：根据输入方向移动 |
| `AttackState` | `AttackState.h` | 攻击状态：播放攻击动画，期间可取消窗口切其他状态 |
| `HurtState` | `HurtState.h` | 受伤硬直：不可操作，计时结束后恢复 |
| `DeadState` | `DeadState.h` | 死亡终态：不可逆，等待 CleanupSystem 销毁 |

---

## 六、Factories 模块（2 个工厂）

| 工厂 | 文件 | 职责 |
|------|------|------|
| `EntityFactory` | `EntityFactory.h` | JSON 驱动实体工厂：从 data/prefabs/*.json 加载配置，支持 spawnPlayer/spawnDummy/spawnThrowableBomb/spawnExplosion |
| `PlayerFactory` | `PlayerFactory.h` | 遗留硬编码工厂：createPlayer/createEnemy/createLoot（已被 EntityFactory 取代） |

---

## 七、Utils 模块（4 个工具）

| 工具 | 文件 | 职责 |
|------|------|------|
| `CollisionTypes` | `CollisionTypes.h` | 碰撞层位掩码（Player/Enemy/Projectile 等）、阵营枚举、碰撞事件配置、碰撞矩阵 |
| `Logging` | `Logging.h` | 组件字符串化（to_string_cmd）、帧调试输出（printFrameDebug） |
| `Math` | `Math.h` | 遗留扇形检测 inSector（已被 MathUtils 取代） |
| `Rect` | `Rect.h` | 遗留碰撞工具（已被 math/Rect.h 取代） |

---

## 八、模块间依赖关系

```
┌───────────────────────────────────────────────────────┐
│                    VisualSandbox.cpp                   │
│                    (主入口 + 帧循环)                    │
├───────────────────────────────────────────────────────┤
│  依赖:                                                 │
│    ├── Core (GameWorld, ECS, EntityFactory)            │
│    ├── Systems (21 个系统按帧顺序调用)                  │
│    ├── Math (Vec2, Rect, MathUtils)                   │
│    └── Utils (CollisionTypes, Logging)                │
└───────────────────────────┬───────────────────────────┘
                            │
                            ▼
┌───────────────────────────────────────────────────────┐
│                    Project2 核心库                      │
├───────────────────────────────────────────────────────┤
│  Systems 依赖:                                         │
│    ├── Components (读取/写入对应的 POD 数据)            │
│    ├── Core (通过 GameWorld 访问组件存储和 ECS)         │
│    ├── Math (Vec2/Rect 数学运算)                      │
│    └── States (StateMachineSystem 通过 IState 接口)    │
├───────────────────────────────────────────────────────┤
│  Components 依赖:                                      │
│    ├── Math (Vec2, Rect)                              │
│    ├── Core (Entity 类型, CollisionTypes)             │
│    └── 无 SFML 依赖                                   │
└───────────────────────────┬───────────────────────────┘
                            │
                            ▼
┌───────────────────────────────────────────────────────┐
│                    外部依赖                             │
│    ├── SFML 3 (仅 VisualSandbox.cpp + RenderSystem)   │
│    ├── ImGui (仅 VisualSandbox.cpp)                   │
│    └── nlohmann/json (EntityFactory, InputManager)     │
└───────────────────────────────────────────────────────┘
```

---

**维护者:** Project2 Team
**最后更新:** 2026-04-15
**下次审查:** 添加新模块或重构现有模块时

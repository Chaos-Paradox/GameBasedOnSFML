# PlayerFactory 使用示例

> **最后更新:** 2026-03-29  
> **文件:** `include/factories/PlayerFactory.h`

---

## 概述

PlayerFactory 提供静态方法用于创建标准的游戏实体（玩家、敌人、掉落物等），遵循纯 ECS 模式。

---

## 创建玩家实体

### 基础用法

```cpp
#include "core/ECS.h"
#include "core/Component.h"
#include "factories/PlayerFactory.h"

int main() {
    // 1. 初始化 ECS 和组件存储
    ECS ecs;
    ComponentStore<TransformComponent> transforms;
    ComponentStore<CharacterComponent> characters;
    ComponentStore<StateMachineComponent> states;
    ComponentStore<InventoryComponent> inventories;
    
    // 2. 创建玩家实体
    EntityId player = PlayerFactory::createPlayer(
        ecs,
        transforms,
        characters,
        states,
        inventories,
        {100.0f, 100.0f}  // 初始位置
    );
    
    // 3. 验证组件已添加
    assert(transforms.has(player));
    assert(characters.has(player));
    assert(states.has(player));
    assert(inventories.has(player));
    
    // 4. 访问组件数据
    auto& transform = transforms.get(player);
    auto& character = characters.get(player);
    auto& state = states.get(player);
    
    std::cout << "Player HP: " << character.currentHP << "\n";
    std::cout << "Player Pos: (" << transform.position.x << ", " 
              << transform.position.y << ")\n";
    
    return 0;
}
```

### 组件默认值

创建的玩家实体会自动拥有以下默认值：

| 组件 | 字段 | 默认值 |
|------|------|--------|
| **Transform** | position | `{100.0f, 100.0f}` (传入参数) |
| | scale | `{1.0f, 1.0f}` |
| | rotation | `0.0f` |
| | velocity | `{0.0f, 0.0f}` |
| **Character** | name | `"Player"` |
| | level | `1` |
| | maxHP | `100` |
| | currentHP | `100` (满血) |
| | baseAttack | `10` |
| | baseDefense | `5` |
| | baseMoveSpeed | `150.0f` |
| | isInvincible | `false` |
| | facingX | `1.0f` (向右) |
| **StateMachine** | currentState | `CharacterState::Idle` |
| | previousState | `CharacterState::Idle` |
| | stateTimer | `0.0f` |
| **Inventory** | slots | `空` (预分配 20 格) |
| | maxSlots | `20` |

---

## 创建敌人实体

```cpp
// 创建普通敌人
EntityId enemy1 = PlayerFactory::createEnemy(
    ecs,
    transforms,
    characters,
    states,
    {200.0f, 100.0f},  // 位置
    50,                 // HP
    8                   // 攻击力
);

// 创建强力敌人
EntityId boss = PlayerFactory::createEnemy(
    ecs,
    transforms,
    characters,
    states,
    {300.0f, 150.0f},
    200,  // 200 HP
    20    // 20 攻击
);
```

---

## 创建掉落物实体

```cpp
// 创建金币掉落
EntityId gold = PlayerFactory::createLoot(
    ecs,
    transforms,
    itemData,
    {150.0f, 120.0f},
    1001,   // 金币物品 ID
    50      // 50 个
);

// 创建药水掉落
EntityId potion = PlayerFactory::createLoot(
    ecs,
    transforms,
    itemData,
    {160.0f, 120.0f},
    2001,   // 药水物品 ID
    1       // 1 个
);
```

---

## 完整游戏初始化示例

```cpp
#include "core/ECS.h"
#include "core/Component.h"
#include "factories/PlayerFactory.h"
#include "systems/StateMachineSystem.h"
#include "systems/CollisionSystem.h"

class Game {
private:
    ECS ecs;
    ComponentStore<TransformComponent> transforms;
    ComponentStore<CharacterComponent> characters;
    ComponentStore<StateMachineComponent> states;
    ComponentStore<InventoryComponent> inventories;
    ComponentStore<InputCommand> inputs;
    ComponentStore<DamageTagComponent> damageTags;
    
    StateMachineSystem stateSystem;
    CollisionSystem collisionSystem;
    
    EntityId player;
    
public:
    void init() {
        // 创建玩家
        player = PlayerFactory::createPlayer(
            ecs,
            transforms,
            characters,
            states,
            inventories,
            {400.0f, 300.0f}
        );
        
        // 创建几个敌人
        for (int i = 0; i < 3; ++i) {
            PlayerFactory::createEnemy(
                ecs,
                transforms,
                characters,
                states,
                {100.0f + i * 150.0f, 200.0f},
                50,
                8
            );
        }
        
        std::cout << "Game initialized with " 
                  << ecs.entities().size() << " entities\n";
    }
    
    void update(float dt) {
        // 更新状态机
        stateSystem.update(states, inputs, damageTags, ecs.entities(), dt);
        
        // 更新碰撞
        collisionSystem.update(
            hitboxes, hurtboxes, transforms, damageTags,
            ecs.entities(),
            currentTime
        );
        
        // 处理玩家输入
        handleInput();
    }
    
    void handleInput() {
        // 简单的输入处理
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) {
            inputs.get(player).cmd = Command::MoveUp;
        } else if (sf::Keyboard::isKeyPressed(sf::Keyboard::J)) {
            inputs.get(player).cmd = Command::Attack;
        } else {
            inputs.get(player).cmd = Command::None;
        }
    }
    
    void render() {
        // 渲染逻辑...
    }
};
```

---

## 自定义玩家配置

如果需要自定义玩家属性，可以在创建后修改：

```cpp
// 创建玩家
EntityId player = PlayerFactory::createPlayer(
    ecs, transforms, characters, states, inventories,
    {100.0f, 100.0f}
);

// 自定义属性
auto& character = characters.get(player);
character.maxHP = 150;
character.currentHP = 150;
character.baseAttack = 15;
character.baseMoveSpeed = 180.0f;

// 或者添加额外组件
hitboxes.add(player, {
    .bounds = {0, 0, 30, 50},
    .active = false
});
```

---

## 内存安全注意事项

### ✅ 正确做法

```cpp
// 使用 std::move 避免拷贝
InventoryComponent inventory;
inventory.slots.reserve(inventory.maxSlots);
inventories.add(entity, std::move(inventory));

// 检查组件是否存在
if (characters.has(entity)) {
    auto& character = characters.get(entity);
    // 安全使用
}
```

### ❌ 错误做法

```cpp
// 直接访问未检查的组件
auto& character = characters.get(entity); // 可能抛出异常

// 忘记初始化组件
transforms.add(entity, {}); // 应该明确指定值

// 悬空引用
auto& transform = transforms.get(entity);
// ... 其他操作可能重新分配内存 ...
transform.position = {0, 0}; // 可能已失效
```

---

## 性能优化建议

### 1. 预分配组件存储

```cpp
ComponentStore<TransformComponent> transforms;
transforms.data.reserve(1000); // 预分配 1000 个实体空间
```

### 2. 批量创建实体

```cpp
// 批量创建敌人
std::vector<EntityId> enemies;
enemies.reserve(10);
for (int i = 0; i < 10; ++i) {
    enemies.push_back(PlayerFactory::createEnemy(...));
}
```

### 3. 使用结构化绑定 (C++17)

```cpp
auto& [pos, scale, rot, vel] = transforms.get(entity);
pos.x += vel.x * dt;
```

---

## 相关文档

- [`01_DATA_SCHEMA.md`](docs/01_DATA_SCHEMA.md) - 数据字典
- [`COMPONENT_LIST.md`](docs/COMPONENT_LIST.md) - Component 清单
- [`MIGRATION_GUIDE.md`](docs/MIGRATION_GUIDE.md) - 迁移指南

---

**维护者:** Project2 Team  
**最后更新:** 2026-03-29

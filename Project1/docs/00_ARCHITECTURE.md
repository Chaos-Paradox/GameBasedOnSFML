# 00_ARCHITECTURE.md - Project1 架构地图

ECS 框架的核心骨架和模块通讯协议。

---

## 系统概览

```
┌─────────────────────────────────────────────────────────────┐
│                        Project1: ECS Core                     │
├─────────────────────────────────────────────────────────────┤
│  ┌──────────────────────────────────────────────────────┐   │
│  │                    World / Scene                      │   │
│  │  ┌────────────┐  ┌────────────┐  ┌────────────┐     │   │
│  │  │  Entity    │  │  Entity    │  │  Entity    │     │   │
│  │  │  (ID+Mask) │  │  (ID+Mask) │  │  (ID+Mask) │ ... │   │
│  │  └─────┬──────┘  └─────┬──────┘  └─────┬──────┘     │   │
│  │        │               │               │             │   │
│  │        ▼               ▼               ▼             │   │
│  │  ┌─────────────────────────────────────────────┐    │   │
│  │  │            Component Stores                  │    │   │
│  │  │  [Transform] [Sprite] [Collision] [AI] ...  │    │   │
│  │  └─────────────────────────────────────────────┘    │   │
│  └──────────────────────────────────────────────────────┘   │
├─────────────────────────────────────────────────────────────┤
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐      │
│  │ RenderSystem │  │PhysicsSystem │  │  InputSystem │      │
│  └──────────────┘  └──────────────┘  └──────────────┘      │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐      │
│  │  AISystem    │  │AnimSystem    │  │  UISystem    │      │
│  └──────────────┘  └──────────────┘  └──────────────┘      │
├─────────────────────────────────────────────────────────────┤
│                    Optional: SFML 3.x Layer                  │
└─────────────────────────────────────────────────────────────┘
```

## 核心概念

### Entity（实体）

- **本质：** 唯一 ID + 组件位掩码
- **职责：** 组件容器，无逻辑
- **生命周期：** 由 World 创建和销毁

```cpp
class Entity {
    uint32_t id;
    std::bitset<MAX_COMPONENTS> mask;
    World* world;
    
    template<typename T>
    T& getComponent();
    
    template<typename T>
    T& addComponent();
    
    template<typename T>
    void removeComponent();
};
```

### Component（组件）

- **本质：** 纯数据结构
- **职责：** 存储状态，无逻辑
- **存储：** 连续内存（缓存友好）

```cpp
struct TransformComponent {
    sf::Vector2f position;
    sf::Vector2f scale;
    float rotation;
    sf::Vector2f velocity;
};

struct SpriteComponent {
    sf::Sprite sprite;
    std::string textureKey;
    bool visible;
    int renderLayer;
};
```

### System（系统）

- **本质：** 处理逻辑
- **职责：** 遍历匹配的实体，更新组件数据
- **执行：** 每帧调用 `update(dt)`

```cpp
class System {
protected:
    World* world;
    std::bitset<MAX_COMPONENTS> requiredMask;
    
public:
    virtual void update(float dt) = 0;
    
    template<typename T>
    T& getComponent(EntityId id);
    
    std::vector<EntityId> getMatchingEntities();
};
```

### World（世界）

- **本质：** ECS 容器
- **职责：** 管理实体、组件存储、系统
- **生命周期：** 游戏启动时创建

```cpp
class World {
    std::queue<EntityId> availableIds;
    std::unordered_map<ComponentType, IComponentStore*> stores;
    std::vector<std::unique_ptr<System>> systems;
    
    Entity createEntity();
    void destroyEntity(EntityId id);
    
    template<typename T>
    void registerComponent();
    
    template<typename T>
    void registerSystem();
    
    void update(float dt);
};
```

## 数据流

### 游戏循环

```
Game::run()
   │
   ├──► handleInput()
   │      └──► InputSystem.update(dt)
   │
   ├──► update(dt)
   │      ├──► PhysicsSystem.update(dt)
   │      ├──► AISystem.update(dt)
   │      ├──► AnimationSystem.update(dt)
   │      └──► ...其他系统
   │
   └──► render()
          └──► RenderSystem.update(dt)
```

### 系统执行顺序

```
1. InputSystem      - 处理输入，设置输入组件
2. PhysicsSystem    - 更新位置、速度、碰撞
3. AISystem         - AI 决策，设置目标
4. AnimationSystem  - 更新动画状态
5. CombatSystem     - 处理攻击、伤害
6. RenderSystem     - 绘制所有可见实体
```

## 组件类型

### 基础组件

| 组件 | 用途 | 数据 |
|------|------|------|
| Transform | 位置、旋转、缩放 | position, scale, rotation, velocity |
| Sprite | 渲染精灵 | sprite, textureKey, visible, layer |
| Animation | 动画播放 | currentAnim, frame, timer, clips |
| Collision | 碰撞检测 | bounds, layer, mask, isTrigger |

### 游戏组件

| 组件 | 用途 | 数据 |
|------|------|------|
| Character | 角色属性 | hp, attack, defense, speed, state |
| AI | AI 控制 | targetType, state, decisionTimer |
| Item | 物品 | itemType, effect, stackSize |
| Projectile | 投射物 | damage, speed, lifetime, sourceId |

## 系统类型

### 基础系统

| 系统 | 依赖组件 | 功能 |
|------|----------|------|
| RenderSystem | Transform, Sprite | 绘制精灵 |
| PhysicsSystem | Transform, Collision | 碰撞检测和响应 |
| InputSystem | Input, Transform | 输入映射到动作 |
| AnimationSystem | Transform, Sprite, Animation | 动画更新 |

### 游戏系统

| 系统 | 依赖组件 | 功能 |
|------|----------|------|
| AISystem | Transform, AI, Character | AI 决策和移动 |
| CombatSystem | Character, Collision | 攻击和伤害计算 |
| SpawnSystem | Spawner, AI | 敌人生成 |
| UISystem | UIElement, Transform | UI 渲染 |

## 通讯模式

### 系统间通讯（通过组件）

```
AISystem                    PhysicsSystem
   │                            │
   │ 1. 设置 velocity           │
   ├───────────────────────────►│
   │                            │
   │                            │ 2. 更新 position
   │                            │
   ◄────────────────────────────┤
   │ 3. 读取 position           │
```

### 事件系统（可选）

```cpp
class EventSystem : public System {
    std::unordered_map<EventType, std::vector<EventHandler>> listeners;
    
    void subscribe(EventType type, EventHandler handler);
    void publish(Event event);
};

// 使用
eventSystem.subscribe(EventType::Collision, [](Entity a, Entity b) {
    // 处理碰撞事件
});
```

## 性能优化

### 内存布局

```
ComponentStore<TransformComponent>:
┌─────────────────────────────────────────────┐
│ Transform[0] │ Transform[1] │ Transform[2] │ ...
│ (连续内存)                                    │
└─────────────────────────────────────────────┘
```

### 缓存友好遍历

```cpp
// ✅ 好：连续内存遍历
for (auto& transform : transformStore) {
    transform.position += transform.velocity * dt;
}

// ❌ 差：间接访问
for (auto entityId : entities) {
    auto* transform = getComponent<Transform>(entityId);
    transform->position += transform->velocity * dt;
}
```

### 系统批处理

```cpp
class RenderSystem : public System {
    void update(float dt) override {
        // 按纹理批次分组
        std::unordered_map<Texture*, std::vector<sf::Sprite>> batches;
        
        for (auto id : getMatchingEntities()) {
            auto& sprite = get<Sprite>(id);
            batches[sprite.texture].push_back(sprite.sprite);
        }
        
        // 批量绘制
        for (auto& [texture, sprites] : batches) {
            window.draw(sprites);  // 单次绘制调用
        }
    }
};
```

## 扩展点

### 添加新组件

1. 在 `include/components/` 创建头文件
2. 定义纯数据结构
3. 在 `World::init()` 注册

### 添加新系统

1. 在 `include/systems/` 创建头文件
2. 继承 `System` 基类
3. 实现 `update(dt)` 方法
4. 在 `World::init()` 注册

### 添加新功能

1. 检查是否需要新组件
2. 检查是否需要新系统
3. 更新 `docs/features/` 文档
4. 编写测试

## 与 Project2 的关系

```
Project1 (ECS 框架)              Project2 (游戏逻辑)
┌─────────────────────┐         ┌─────────────────────┐
│ World               │         │ CharacterStateMachine│
│ ├── Entity          │         │ ├── IdleState       │
│ ├── ComponentStore  │◄────────┤ ├── MoveState       │
│ └── System          │  使用   │ └── AttackState     │
└─────────────────────┘         └─────────────────────┘
         │                                  │
         └────────── 共享组件 ──────────────┘
                    (CharacterComponent)
```

- **Project1:** 通用 ECS 框架，可复用
- **Project2:** 特定游戏逻辑，依赖 Project1

---

**维护者：** Project1 Team  
**最后更新：** 2026-03-29  
**版本：** 1.0

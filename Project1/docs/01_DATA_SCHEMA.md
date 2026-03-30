# 01_DATA_SCHEMA.md - Project1 数据字典

所有核心数据结构和组件定义。

---

## 核心类型

### EntityId

```cpp
using EntityId = uint32_t;
constexpr EntityId INVALID_ENTITY_ID = UINT32_MAX;
```

### ComponentType

```cpp
enum class ComponentType : uint8_t {
    None = 0,
    
    // 基础组件
    Transform,
    Sprite,
    Animation,
    Collision,
    Input,
    
    // 游戏组件
    Character,
    AI,
    Item,
    Projectile,
    Spawner,
    
    // UI 组件
    UIElement,
    Text,
    
    // 系统组件
    Camera,
    Light,
    Particle,
    
    COUNT
};
```

### SystemType

```cpp
enum class SystemType : uint8_t {
    None = 0,
    Render,
    Physics,
    Input,
    Animation,
    AI,
    Combat,
    Spawn,
    UI,
    COUNT
};
```

---

## 基础组件

### TransformComponent

```cpp
struct TransformComponent {
    // 位置
    sf::Vector2f position{0.0f, 0.0f};
    
    // 缩放（默认 1,1）
    sf::Vector2f scale{1.0f, 1.0f};
    
    // 旋转（弧度，默认 0）
    float rotation{0.0f};
    
    // 速度（像素/秒）
    sf::Vector2f velocity{0.0f, 0.0f};
    
    // 加速度（像素/秒²）
    sf::Vector2f acceleration{0.0f, 0.0f};
    
    // 本地边界（用于碰撞）
    sf::FloatRect localBounds{0.0f, 0.0f, 32.0f, 32.0f};
    
    // 标记
    bool isStatic{false};      // 是否静态（不受物理影响）
    bool isKinematic{false};   // 是否运动学（手动控制位置）
    
    // 工具方法
    sf::Vector2f getCenter() const {
        return {position.x + localBounds.width / 2, 
                position.y + localBounds.height / 2};
    }
    
    sf::FloatRect getGlobalBounds() const {
        return {position.x, position.y, 
                localBounds.width * scale.x, 
                localBounds.height * scale.y};
    }
};
```

### SpriteComponent

```cpp
struct SpriteComponent {
    // SFML 精灵
    sf::Sprite sprite;
    
    // 纹理资源键
    std::string textureKey;
    
    // 可见性
    bool visible{true};
    
    // 渲染层级（数字小的先绘制）
    int renderLayer{0};
    
    // 颜色调制（用于特效）
    sf::Color colorModulation{255, 255, 255, 255};
    
    // 翻转
    bool flipX{false};
    bool flipY{false};
    
    // 原点（归一化 0-1）
    sf::Vector2f origin{0.5f, 0.5f};
    
    void updateSprite() {
        sprite.setTexture(*textureManager.get(textureKey));
        sprite.setPosition(position);
        sprite.setScale(scale.x * (flipX ? -1 : 1), 
                       scale.y * (flipY ? -1 : 1));
        sprite.setRotation(rotation * 180.0f / 3.14159f);
        sprite.setColor(colorModulation);
    }
};
```

### AnimationComponent

```cpp
struct AnimationClip {
    std::string name;
    std::vector<sf::IntRect> frames;
    float duration;           // 总持续时间（秒）
    bool loop{true};
};

struct AnimationComponent {
    // 动画剪辑库
    std::unordered_map<std::string, AnimationClip> clips;
    
    // 当前播放
    std::string currentClip;
    bool isPlaying{false};
    bool isPaused{false};
    
    // 播放状态
    float currentTime{0.0f};
    int currentFrame{0};
    
    // 播放速度
    float speedMultiplier{1.0f};
    
    // 工具方法
    void play(const std::string& name) {
        if (clips.find(name) != clips.end()) {
            currentClip = name;
            currentTime = 0.0f;
            currentFrame = 0;
            isPlaying = true;
            isPaused = false;
        }
    }
    
    void stop() {
        isPlaying = false;
        currentTime = 0.0f;
        currentFrame = 0;
    }
    
    void pause() { isPaused = true; }
    void resume() { isPaused = false; }
    
    void update(float dt) {
        if (!isPlaying || isPaused) return;
        
        auto& clip = clips[currentClip];
        currentTime += dt * speedMultiplier;
        
        if (currentTime >= clip.duration) {
            if (clip.loop) {
                currentTime = std::fmod(currentTime, clip.duration);
            } else {
                currentTime = clip.duration;
                isPlaying = false;
            }
        }
        
        // 计算当前帧
        float frameProgress = currentTime / clip.duration;
        currentFrame = static_cast<int>(frameProgress * clip.frames.size());
        currentFrame = std::min(currentFrame, static_cast<int>(clip.frames.size()) - 1);
    }
    
    sf::IntRect getCurrentFrame() const {
        if (clips.find(currentClip) == clips.end()) {
            return {0, 0, 32, 32};
        }
        return clips.at(currentClip).frames[currentFrame];
    }
};
```

### CollisionComponent

```cpp
struct CollisionComponent {
    // 碰撞层级（位掩码）
    enum class Layer : uint16_t {
        None      = 0,
        Player    = 1 << 0,
        Enemy     = 1 << 1,
        Obstacle  = 1 << 2,
        Trigger   = 1 << 3,
        Attack    = 1 << 4,
        Item      = 1 << 5,
        Projectile= 1 << 6,
        All       = 0xFFFF
    };
    
    // 自身层级
    Layer layer{Layer::None};
    
    // 检测掩码（与哪些层级碰撞）
    uint16_t mask{static_cast<uint16_t>(Layer::All)};
    
    // 碰撞边界（世界坐标）
    sf::FloatRect bounds{0.0f, 0.0f, 32.0f, 32.0f};
    
    // 是否为触发器（不产生物理碰撞）
    bool isTrigger{false};
    
    // 碰撞回调
    std::function<void(EntityId self, EntityId other)> onCollisionEnter;
    std::function<void(EntityId self, EntityId other)> onCollisionExit;
    
    // 工具方法
    bool shouldCollideWith(Layer other) const {
        return (static_cast<uint16_t>(layer) & static_cast<uint16_t>(other)) != 0;
    }
    
    static bool checkOverlap(const sf::FloatRect& a, const sf::FloatRect& b) {
        return a.left < b.left + b.width &&
               a.left + a.width > b.left &&
               a.top < b.top + b.height &&
               a.top + a.height > b.top;
    }
};
```

---

## 游戏组件

### CharacterComponent

```cpp
struct CharacterComponent {
    // 基础属性
    std::string name;
    int level{1};
    
    // 战斗属性
    int maxHP{100};
    int currentHP{100};
    int attack{10};
    int defense{5};
    
    // 移动属性
    float moveSpeed{150.0f};      // 像素/秒
    float runMultiplier{1.8f};    // 奔跑倍率
    
    // 状态
    enum class State : uint8_t {
        Idle,
        Move,
        Attack,
        Hurt,
        Dead
    };
    State state{State::Idle};
    float stateTimer{0.0f};
    
    // 标志
    bool isGrounded{true};
    bool isAttacking{false};
    bool isInvincible{false};
    float invincibleTimer{0.0f};
    
    // 方向
    enum class Facing : uint8_t {
        Left,
        Right,
        Up,
        Down
    };
    Facing facing{Facing::Right};
    
    // 冷却
    float attackCooldown{0.0f};
    float attackCooldownMax{0.5f};
    
    // 工具方法
    bool isAlive() const { return currentHP > 0; }
    
    void takeDamage(int amount) {
        if (isInvincible) return;
        currentHP = std::max(0, currentHP - amount);
        if (currentHP <= 0) {
            state = State::Dead;
        }
    }
};
```

### AIComponent

```cpp
struct AIComponent {
    // AI 类型
    enum class Type : uint8_t {
        Idle,           // 待机
        Patrol,         // 巡逻
        Chase,          // 追击
        Attack,         // 攻击
        Flee            // 逃跑
    };
    Type type{Type::Idle};
    
    // 当前状态
    enum class State : uint8_t {
        Roam,
        Alert,
        Combat,
        Return
    };
    State state{State::Roam};
    
    // 目标
    EntityId targetId{INVALID_ENTITY_ID};
    sf::Vector2f targetPosition{0.0f, 0.0f};
    
    // 巡逻点
    std::vector<sf::Vector2f> patrolPoints;
    int currentPatrolIndex{0};
    
    // 感知范围
    float detectionRange{200.0f};
    float attackRange{50.0f};
    
    // 决策计时器
    float decisionTimer{0.0f};
    float decisionInterval{0.5f};
    
    // 配置
    bool isActive{true};
    float reactionTime{0.2f};
};
```

### ItemComponent

```cpp
struct ItemComponent {
    // 物品类型
    enum class Type : uint8_t {
        Consumable,     // 消耗品
        Equipment,      // 装备
        Material,       // 材料
        Quest,          // 任务物品
        Currency        // 货币
    };
    Type type{Type::Consumable};
    
    // 物品 ID
    std::string itemId;
    
    // 堆叠
    int stackSize{1};
    int maxStackSize{99};
    
    // 效果（消耗品）
    struct Effect {
        enum class Target { Self, Player, Area };
        Target target{Target::Self};
        int hpRestore{0};
        int mpRestore{0};
        float duration{0.0f};
        std::string buffId;
    };
    Effect effect;
    
    // 拾取
    bool canPickup{true};
    float respawnTime{0.0f};
};
```

### ProjectileComponent

```cpp
struct ProjectileComponent {
    // 来源
    EntityId sourceId{INVALID_ENTITY_ID};
    
    // 伤害
    int damage{10};
    
    // 运动
    sf::Vector2f velocity{0.0f, 0.0f};
    float speed{300.0f};
    
    // 生命周期
    float lifetime{2.0f};
    float elapsed{0.0f};
    
    // 穿透
    int pierceCount{1};
    std::set<EntityId> hitEntities;
    
    // 类型
    enum class Type : uint8_t {
        Physical,
        Magical,
        Arrow,
        Bullet,
        Magic
    };
    Type projectileType{Type::Physical};
    
    void update(float dt) {
        elapsed += dt;
    }
    
    bool isExpired() const {
        return elapsed >= lifetime || pierceCount <= 0;
    }
};
```

---

## 配置结构

### GameConfig

```cpp
struct GameConfig {
    struct Window {
        uint32_t width{1920};
        uint32_t height{1080};
        std::string title{"SFMLGame"};
        uint32_t framerateLimit{60};
        bool vsync{true};
        bool fullscreen{false};
    } window;
    
    struct Debug {
        bool showFPS{true};
        bool showEntityCount{true};
        bool showCollisionBounds{false};
        bool showComponentDetails{false};
    } debug;
    
    struct Paths {
        std::string materialDir{"material/"};
        std::string textureDir{"material/pictures/"};
        std::string fontDir{"material/fonts/"};
        std::string audioDir{"material/audio/"};
    } paths;
};
```

---

## 常量定义

```cpp
// 实体限制
constexpr uint32_t MAX_ENTITIES = 4096;
constexpr uint32_t MAX_COMPONENTS = 64;
constexpr uint32_t MAX_SYSTEMS = 32;

// 时间常量
constexpr float FIXED_TIMESTEP = 1.0f / 60.0f;
constexpr float MAX_FRAME_TIME = 0.25f;

// 渲染层级
constexpr int LAYER_BACKGROUND = 0;
constexpr int LAYER_GROUND = 10;
constexpr int LAYER_OBJECTS = 20;
constexpr int LAYER_CHARACTERS = 30;
constexpr int LAYER_EFFECTS = 40;
constexpr int LAYER_UI = 100;

// 默认值
constexpr int DEFAULT_MAX_HP = 100;
constexpr float DEFAULT_MOVE_SPEED = 150.0f;
constexpr float DEFAULT_ATTACK_COOLDOWN = 0.5f;
```

---

**维护者：** Project1 Team  
**最后更新：** 2026-03-29  
**版本：** 1.0

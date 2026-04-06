/**
 * @file VisualSandbox.cpp
 * @brief 可视化调试沙盒 - 战斗管线测试（含冲刺与战利品系统）
 */

#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <iostream>
#include <string>
#include <vector>
#include <cmath>
#include <algorithm>

#include "core/ECS.h"
#include "core/Entity.h"
#include "components/Transform.h"
#include "components/StateMachine.h"
#include "components/Character.h"
#include "components/InputCommand.h"
#include "components/Hurtbox.h"
#include "components/Hitbox.h"
#include "components/Lifetime.h"
#include "components/DamageTag.h"
#include "components/AttackState.h"
#include "components/DeathTag.h"
#include "components/LootDrop.h"
#include "components/ItemData.h"
#include "components/PickupBox.h"
#include "components/Evolution.h"
#include "components/MagnetComponent.h"
#include "components/DashComponent.h"
#include "components/DamageEventComponent.h"
#include "components/ZTransformComponent.h"
#include "components/ColliderComponent.h"
#include "components/AttachedComponent.h"
#include "components/BombComponent.h"
#include "components/DeathTag.h"

#include "systems/StateMachineSystem.h"
#include "systems/LocomotionSystem.h"
#include "systems/MovementSystem.h"
#include "systems/AttackSystem.h"
#include "systems/CollisionSystem.h"
#include "systems/DamageSystem.h"
#include "systems/DeathSystem.h"
#include "systems/LootSpawnSystem.h"
#include "systems/PickupSystem.h"
#include "systems/MagnetSystem.h"
#include "systems/CleanupSystem.h"
#include "systems/DebugSystem.h"
#include "systems/DashSystem.h"
#include "systems/PhysicalCollisionSystem.h"
#include "systems/AttachmentSystem.h"
#include "systems/BombSystem.h"
#include "systems/CleanupSystem.h"

constexpr int WINDOW_WIDTH = 1024;
constexpr int WINDOW_HEIGHT = 768;
constexpr float ENTITY_SIZE = 40.0f;

sf::Color COLOR_PLAYER(50, 200, 50);
sf::Color COLOR_ENEMY(200, 50, 50);
sf::Color COLOR_HURT(255, 100, 100);
sf::Color COLOR_DEAD(100, 100, 100);
sf::Color COLOR_HITBOX(255, 255, 0, 128);
sf::Color COLOR_LOOT(50, 255, 50); 
sf::Color COLOR_BACKGROUND(30, 30, 30);

std::string stateToString(CharacterState state) {
    switch(state) {
        case CharacterState::Idle: return "IDLE";
        case CharacterState::Move: return "MOVE";
        case CharacterState::Dash: return "DASH";
        case CharacterState::Attack: return "ATTACK";
        case CharacterState::Hurt: return "HURT";
        case CharacterState::Dead: return "DEAD";
        default: return "UNKNOWN";
    }
}

Vec2 getInputFromKeyboard() {
    Vec2 dir{0.0f, 0.0f};
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up)) dir.y -= 1.0f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down)) dir.y += 1.0f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left)) dir.x -= 1.0f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right)) dir.x += 1.0f;
    return dir;
}

auto createPlayer(ECS& ecs, ComponentStore<StateMachineComponent>& states, ComponentStore<TransformComponent>& transforms,
                  ComponentStore<CharacterComponent>& characters, ComponentStore<InputCommand>& inputs,
                  ComponentStore<HurtboxComponent>& hurtboxes, ComponentStore<EvolutionComponent>& evolutions,
                  ComponentStore<DashComponent>& dashes, ComponentStore<MagnetComponent>& magnets,
                  ComponentStore<ZTransformComponent>& zTransforms,
                  ComponentStore<ColliderComponent>& colliders,  // ← 新增参数
                  float x, float y) {
    auto player = ecs.create();
    states.add(player, {CharacterState::Idle, CharacterState::Idle, 0.0f});
    transforms.add(player, {{x, y}, {1.0f, 1.0f}, 0.0f, {0.0f, 0.0f}, 1.0f, 0.0f});
    characters.add(player, {"Player", 1, 100, 100, 10, 5, 200.0f, false, 0.0f, 1.0f, 0.0f});
    inputs.add(player, {{0.0f, 0.0f}, ActionIntent::None, 0.0f});
    hurtboxes.add(player, {20.0f, {0.0f, 0.0f}, Faction::Player, 1, 0.0f});  // radius=20px, offset={0,0}
    evolutions.add(player, {0, 0});
    
    // 赋予玩家冲刺能力
    dashes.add(player, {
        .dashSpeed = 2000.0f, 
        .dashDuration = 0.1f,
        .iframeDuration = 0.1f,
        .cooldown = 1.0f, 
        .dashTimer = 0.0f, 
        .cooldownTimer = 0.0f,
        .iframeTimer = 0.0f,
        .dashDir = {1.0f, 0.0f},
        .isInvincible = false
    });
    
    // 赋予玩家磁吸能力
    magnets.add(player, {
        .magnetRadius = 150.0f,
        .magnetSpeed = 400.0f
    });
    
    // 赋予玩家 Z 轴能力（可以跳跃）
    zTransforms.add(player, {
        .z = 0.0f,
        .vz = 0.0f,
        .gravity = -2000.0f,
        .height = 40.0f
    });
    
    // ← 新增：赋予玩家圆柱体碰撞器（动态实体）
    colliders.add(player, {
        .radius = 20.0f,
        .isStatic = false
    });
    
    return player;
}

auto createDummy(ECS& ecs, ComponentStore<StateMachineComponent>& states, ComponentStore<TransformComponent>& transforms,
                 ComponentStore<CharacterComponent>& characters, ComponentStore<HurtboxComponent>& hurtboxes,
                 ComponentStore<LootDropComponent>& lootDrops,
                 ComponentStore<ColliderComponent>& colliders,  // ← 新增参数
                 float x, float y) {
    auto dummy = ecs.create();
    states.add(dummy, {CharacterState::Idle, CharacterState::Idle, 0.0f});
    transforms.add(dummy, {{x, y}, {1.0f, 1.0f}, 0.0f, {0.0f, 0.0f}, -1.0f, 0.0f});
    characters.add(dummy, {"Dummy", 1, 100, 100, 10, 0, 0.0f, false, 0.0f, -1.0f, 0.0f});
    hurtboxes.add(dummy, {20.0f, {0.0f, 0.0f}, Faction::Enemy, 2, 0.0f});  // radius=20px, offset={0,0}
    
    LootDropComponent dummyLoot;
    dummyLoot.lootTable[0] = {1, 1.0f, 1, 1};
    dummyLoot.lootCount = 1;
    dummyLoot.hasDropped = false;
    lootDrops.add(dummy, dummyLoot);
    
    // ← 新增：赋予假人圆柱体碰撞器（动态实体）
    colliders.add(dummy, {
        .radius = 20.0f,
        .isStatic = false
    });
    
    return dummy;
}

// 渲染实体（支持 Z 轴视觉偏移 + 通用影子）
void renderEntity(sf::RenderWindow& window, const TransformComponent& trans, const CharacterComponent& chara,
                  const StateMachineComponent& state, bool isPlayer, const DashComponent* dash = nullptr,
                  const ZTransformComponent* zComp = nullptr) {
    // ← 【2.5D 通用影子】永远绘制在逻辑坐标上
    sf::CircleShape shadow(30.0f);        // ← 影子大一倍（从 15 改为 30）
    shadow.setOrigin({30.0f, 15.0f});     // 中心点偏移
    shadow.setScale({1.0f, 0.5f});        // 压扁成 2.5D 椭圆形
    shadow.setFillColor(sf::Color(0, 0, 0, 100));  // 半透明黑色
    shadow.setPosition({trans.position.x, trans.position.y});
    window.draw(shadow);
    
    sf::RectangleShape rect({ENTITY_SIZE, ENTITY_SIZE});
    rect.setOrigin({ENTITY_SIZE / 2.0f, ENTITY_SIZE / 2.0f});
    
    // 视觉 Y = 逻辑 Y - Z（空中实体要往上画）
    float renderX = trans.position.x;
    float renderY = trans.position.y;
    if (zComp && zComp->z > 0.0f) {
        renderY -= zComp->z;
    }
    
    rect.setPosition({renderX, renderY});
    
    sf::Color entityColor = isPlayer ? COLOR_PLAYER : COLOR_ENEMY;

    // 视觉反馈：渲染冲刺和无敌帧
    if (state.currentState == CharacterState::Dead) {
        entityColor = COLOR_DEAD;
    } else if (state.currentState == CharacterState::Hurt) {
        entityColor = COLOR_HURT;
    } else if (state.currentState == CharacterState::Dash && dash != nullptr) {
        if (dash->isInvincible) {
            entityColor = sf::Color(0, 255, 255, 180); // 无敌期间显示半透明青色！
        } else {
            entityColor = sf::Color(isPlayer ? 50 : 200, 100, 100, 200); // 冲刺后摇
        }
    }

    rect.setFillColor(entityColor);
    window.draw(rect);
    
    float hpBarWidth = 40.0f;
    float hpBarHeight = 5.0f;
    float hpBarY = trans.position.y - ENTITY_SIZE / 2.0f - 8.0f;
    
    sf::RectangleShape hpBarBg({hpBarWidth, hpBarHeight});
    hpBarBg.setOrigin({hpBarWidth / 2.0f, hpBarHeight / 2.0f});
    hpBarBg.setPosition({trans.position.x, hpBarY});
    hpBarBg.setFillColor(sf::Color(200, 0, 0));
    window.draw(hpBarBg);
    
    float hpPercent = static_cast<float>(chara.currentHP) / std::max(1, chara.maxHP);
    sf::RectangleShape hpBarFg({hpBarWidth * hpPercent, hpBarHeight});
    hpBarFg.setOrigin({0.0f, hpBarHeight / 2.0f});
    hpBarFg.setPosition({trans.position.x - hpBarWidth / 2.0f, hpBarY});
    hpBarFg.setFillColor(sf::Color(0, 255, 0));
    window.draw(hpBarFg);
}

void renderHitboxes(sf::RenderWindow& window, const ComponentStore<TransformComponent>& transforms,
                    const ComponentStore<HitboxComponent>& hitboxes,
                    const ComponentStore<ZTransformComponent>& zTransforms) {  // ← 新增参数
    for (Entity entity : hitboxes.entityList()) {
        if (!transforms.has(entity)) continue;
        const auto& transform = transforms.get(entity);
        const auto& hitbox = hitboxes.get(entity);
        
        // ← 读取 Z 高度（如果没有则为 0）
        float z = zTransforms.has(entity) ? zTransforms.get(entity).z : 0.0f;
        
        // ← 【核心视觉偏移】渲染坐标 = 实体位置 + Hitbox 局部偏移 - Z 高度
        float centerX = transform.position.x + hitbox.offset.x;
        float centerY = transform.position.y + hitbox.offset.y - z;
        
        // 渲染圆形 Hitbox
        sf::CircleShape circle(hitbox.radius);
        circle.setOrigin({hitbox.radius, hitbox.radius});
        circle.setPosition({centerX, centerY});
        circle.setFillColor(sf::Color(255, 255, 0, 128));  // 半透明黄色
        window.draw(circle);
    }
}

void renderLoot(sf::RenderWindow& window, const ComponentStore<TransformComponent>& transforms,
                const ComponentStore<ItemDataComponent>& itemDatas) {
    for (Entity loot : itemDatas.entityList()) {
        if (!transforms.has(loot)) continue;
        const auto& transform = transforms.get(loot);
        sf::CircleShape lootCircle(8.0f);
        lootCircle.setOrigin({8.0f, 8.0f});
        lootCircle.setPosition({transform.position.x, transform.position.y});
        lootCircle.setFillColor(COLOR_LOOT);
        lootCircle.setOutlineColor(sf::Color(0, 150, 0));
        lootCircle.setOutlineThickness(2.0f);
        window.draw(lootCircle);
    }
}

// ← 新增：渲染炸弹（黑色球体）
void renderBombs(sf::RenderWindow& window, const ComponentStore<TransformComponent>& transforms,
                 const ComponentStore<BombComponent>& bombs,
                 const ComponentStore<ZTransformComponent>& zTransforms) {
    for (Entity entity : bombs.entityList()) {
        if (!transforms.has(entity)) continue;
        const auto& transform = transforms.get(entity);
        const auto& bomb = bombs.get(entity);
        
        // 读取 Z 高度
        float z = zTransforms.has(entity) ? zTransforms.get(entity).z : 0.0f;
        
        // 渲染坐标 = 实体位置 - Z 高度
        float centerX = transform.position.x;
        float centerY = transform.position.y - z;
        
        // 渲染黑色球体（炸弹）
        sf::CircleShape bombCircle(15.0f);  // 炸弹半径 15px
        bombCircle.setOrigin({15.0f, 15.0f});
        bombCircle.setPosition({centerX, centerY});
        bombCircle.setFillColor(sf::Color(0, 0, 0));  // 黑色
        
        // 添加引信效果（红色小点闪烁）
        float blinkAlpha = 150.0f + 100.0f * std::sin(bomb.fuseTimer * 10.0f);
        sf::CircleShape fuse(4.0f);
        fuse.setOrigin({4.0f, 4.0f});
        fuse.setPosition({centerX + 10.0f, centerY - 10.0f});
        fuse.setFillColor(sf::Color(255, 0, 0, static_cast<std::uint8_t>(blinkAlpha)));
        
        window.draw(bombCircle);
        window.draw(fuse);
    }
}

void renderGrid(sf::RenderWindow& window) {
    for (float x = 0; x < WINDOW_WIDTH; x += 50) {
        sf::VertexArray line(sf::PrimitiveType::Lines, 2);
        line[0].position = {x, 0}; line[1].position = {x, (float)WINDOW_HEIGHT};
        window.draw(line);
    }
    for (float y = 0; y < WINDOW_HEIGHT; y += 50) {
        sf::VertexArray line(sf::PrimitiveType::Lines, 2);
        line[0].position = {0, y}; line[1].position = {(float)WINDOW_WIDTH, y};
        window.draw(line);
    }
}

int main() {
    std::cout << "=== Project2 Fixed Timestep & Dash Sandbox ===\n";
    
    sf::RenderWindow window(sf::VideoMode({WINDOW_WIDTH, WINDOW_HEIGHT}), "Project2 Fixed Timestep Sandbox");
    window.setFramerateLimit(144); // 渲染帧率
    
    // ← 【恢复】帧暂停和渐进功能
    float timeScale = 1.0f;  // 时间缩放（1.0 = 正常，0.0 = 暂停）
    bool frameStep = false;  // 单帧步进标志
    
    // ECS 初始化
    ECS ecs;
    ComponentStore<StateMachineComponent> states;
    ComponentStore<TransformComponent> transforms;
    ComponentStore<CharacterComponent> characters;
    ComponentStore<InputCommand> inputs;
    ComponentStore<HurtboxComponent> hurtboxes;
    ComponentStore<AttackStateComponent> attackStates;
    ComponentStore<HitboxComponent> hitboxes;
    ComponentStore<LifetimeComponent> lifetimes;
    ComponentStore<DamageTag> damageTags;
    ComponentStore<DamageEventComponent> damageEvents;
    ComponentStore<DeathTag> deathTags;
    ComponentStore<LootDropComponent> lootDrops;
    ComponentStore<ItemDataComponent> itemDatas;
    ComponentStore<PickupBoxComponent> pickupBoxes;
    ComponentStore<MagnetComponent> magnets;
    ComponentStore<EvolutionComponent> evolutions;
    ComponentStore<DashComponent> dashes;
    ComponentStore<ZTransformComponent> zTransforms;  // ← 新增：Z 轴组件
    ComponentStore<ColliderComponent> colliders;  // ← 新增：圆柱体碰撞器
    ComponentStore<AttachedComponent> attachedComponents;  // ← 新增：依附组件
    ComponentStore<BombComponent> bombs;  // ← 新增：炸弹组件

    // System 初始化
    StateMachineSystem stateSystem;
    LocomotionSystem locomotionSystem;
    MovementSystem movementSystem;
    AttackSystem attackSystem;
    CollisionSystem collisionSystem;
    DamageSystem damageSystem;
    DeathSystem deathSystem;
    LootSpawnSystem lootSpawnSystem;
    PickupSystem pickupSystem;
    MagnetSystem magnetSystem;
    CleanupSystem cleanupSystem;
    DebugSystem debugSystem;
    DashSystem dashSystem;
    PhysicalCollisionSystem physicalCollisionSystem;  // ← 新增：物理碰撞系统
    AttachmentSystem attachmentSystem;  // ← 新增：依附同步系统
    BombSystem bombSystem;  // ← 新增：炸弹系统

    auto player = createPlayer(ecs, states, transforms, characters, inputs, hurtboxes, evolutions, dashes, magnets, zTransforms, colliders, 200, 300);
    auto dummy = createDummy(ecs, states, transforms, characters, hurtboxes, lootDrops, colliders, 700, 300);
    
    // 🚀 神圣时间常数：锁死 60 Hz 物理运算
    const sf::Time TIME_PER_FRAME = sf::seconds(1.0f / 60.0f);
    sf::Time timeSinceLastUpdate = sf::Time::Zero;
    sf::Clock clock, debugClock;
    
    // 输入缓存变量
    bool lastJPressed = false;
    bool lastSpacePressed = false;
    bool lastBombPressed = false;  // ← 新增：炸弹输入缓存
    
    while (window.isOpen()) {
        sf::Time dtTime = clock.restart();
        timeSinceLastUpdate += dtTime;
        
        while (const auto event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) window.close();
            
            // ← 【修复】ESC 键退出游戏
            if (const auto* kp = event->getIf<sf::Event::KeyPressed>()) {
                if (kp->code == sf::Keyboard::Key::Escape) window.close();
                
                // ← 【恢复】帧暂停和渐进功能
                if (kp->code == sf::Keyboard::Key::P) {
                    timeScale = (timeScale == 0.0f) ? 1.0f : 0.0f;  // 暂停/继续
                    std::cout << "[TimeScale] " << (timeScale == 0.0f ? "Paused" : "Running") << "\n";
                }
                if (kp->code == sf::Keyboard::Key::RBracket) {  // ] 键
                    frameStep = true;  // 单帧步进
                    timeScale = 1.0f;  // 临时恢复时间
                }
            }
            
            // ← 右键点击生成假人
            if (const auto* mb = event->getIf<sf::Event::MouseButtonPressed>()) {
                if (mb->button == sf::Mouse::Button::Right) {
                    sf::Vector2f worldPos = window.mapPixelToCoords(mb->position);
                    createDummy(ecs, states, transforms, characters, hurtboxes, lootDrops, colliders, worldPos.x, worldPos.y);
                    std::cout << "Dummy created at (" << worldPos.x << ", " << worldPos.y << ")\n";
                }
            }
        }
        
        // --- 循环外：抓取瞬时输入 ---
        inputs.get(player).moveDir = getInputFromKeyboard();
        
        // ← 【工业级】单轨覆盖指令槽：Last-In-Wins 原则
        // 核心设计：新指令无条件覆盖旧指令，并重置 0.2 秒保质期
        // 这样即使同时按 J+Space，也会以最后按下的为准，避免逻辑死锁
        bool currentJPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::J);
        if (currentJPressed && !lastJPressed) {
            inputs.get(player).pendingIntent = ActionIntent::Attack;
            inputs.get(player).intentTimer = 0.2f;  // 0.2 秒保质期
        }
        lastJPressed = currentJPressed;

        bool currentSpacePressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space);
        if (currentSpacePressed && !lastSpacePressed) {
            // ← 冲刺指令覆盖攻击指令（Last-In-Wins）
            inputs.get(player).pendingIntent = ActionIntent::Dash;
            inputs.get(player).intentTimer = 0.2f;  // 重置保质期
            // 注意：DashSystem 直接从 inputs 读取意图，不需要额外信号
        }
        lastSpacePressed = currentSpacePressed;
        
        // ← 新增：跳跃输入（K 键）
        bool jumpPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::K);
        if (jumpPressed && zTransforms.has(player)) {
            auto& zComp = zTransforms.get(player);
            if (zComp.isGrounded()) {
                zComp.jump(800.0f);  // ← 跳跃高度翻倍（从 400 改为 800）
                std::cout << "[Jump] Player jumped! vz=800\n";
            }
        }
        
        // ← 新增：丢炸弹输入（G 键）
        bool bombPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::G);
        if (bombPressed && !lastBombPressed) {
            // 生成炸弹实体
            Entity bomb = ecs.create();
            
            const auto& playerTrans = transforms.get(player);
            const auto& playerZ = zTransforms.get(player);
            
            // ← 【修复】获取朝向（如果 facing 为 0，使用移动方向或默认向右）
            float facingX = playerTrans.facingX;
            float facingY = playerTrans.facingY;
            
            if (facingX == 0.0f && facingY == 0.0f) {
                // 使用移动方向
                const auto& input = inputs.get(player);
                if (input.moveDir.x != 0.0f || input.moveDir.y != 0.0f) {
                    facingX = input.moveDir.x;
                    facingY = input.moveDir.y;
                    // 归一化
                    float len = std::sqrt(facingX * facingX + facingY * facingY);
                    if (len > 0.0f) {
                        facingX /= len;
                        facingY /= len;
                    }
                } else {
                    // 默认向右
                    facingX = 1.0f;
                    facingY = 0.0f;
                }
            }
            
            // ← 【修复】生成位置在玩家面前 35 像素处（避免物理粘连）
            float offsetX = facingX * 35.0f;
            float offsetY = facingY * 35.0f;
            
            transforms.add(bomb, {
                .position = {playerTrans.position.x + offsetX, playerTrans.position.y + offsetY},
                .scale = {1.0f, 1.0f},
                .rotation = 0.0f,
                .velocity = {0.0f, 0.0f}  // 无水平速度
            });
            
            bombs.add(bomb, {
                .fuseTimer = 3.0f,  // 3 秒爆炸
                .isKicked = false
            });
            
            // 关键弹跳初速：出生时向上跃起一下
            zTransforms.add(bomb, {
                .z = 20.0f,          // 初始高度 20px
                .vz = 300.0f,        // 向上初速度 300px/s
                .gravity = -1500.0f, // 重力加速度
                .height = 30.0f      // 炸弹物理高度
            });
            
            // ← 【修复】添加物理碰撞体（半径 12px，可被推开）
            colliders.add(bomb, {
                .radius = 12.0f,     // 炸弹碰撞半径
                .isStatic = false
            });
            
            std::cout << "[Bomb] 丢出炸弹！fuse=3.0s pos=(" 
                      << (playerTrans.position.x + offsetX) << ", "
                      << (playerTrans.position.y + offsetY) << ")\n";
        }
        lastBombPressed = bombPressed;

        // --- 神圣的 Fixed Timestep 物理循环 ---
        while (timeSinceLastUpdate >= TIME_PER_FRAME) {
            timeSinceLastUpdate -= TIME_PER_FRAME;
            float fixedDt = TIME_PER_FRAME.asSeconds() * timeScale;  // ← 应用 timeScale
            
            // 单帧步进逻辑
            if (frameStep) {
                frameStep = false;
                timeScale = 0.0f;  // 步进后自动暂停
                std::cout << "[FrameStep] Single frame executed\n";
            }
            
            // 严格按照时序执行管线！
            stateSystem.update(states, attackStates, inputs, damageEvents, ecs, fixedDt);
            
            // 冲刺系统从单轨指令槽读取 Dash 意图
            dashSystem.update(dashes, states, transforms, inputs, fixedDt);
            
            // 基础移动（内部有 if(state == Dash) return 保护）
            locomotionSystem.update(states, transforms, characters, inputs, fixedDt);
            
            magnetSystem.update(transforms, magnets, transforms, itemDatas, fixedDt);
            
            // ← 新增：Z 轴物理更新（应用重力）
            for (Entity entity : zTransforms.entityList()) {
                if (zTransforms.has(entity)) {
                    zTransforms.get(entity).applyGravity(fixedDt);
                }
            }
            
            // 位移执行系统（将冲刺速度化为实质距离）
            movementSystem.update(transforms, itemDatas, fixedDt);
            
            // ← 新增：物理碰撞系统（圆柱体排斥，在 Movement 之后，战斗碰撞之前）
            physicalCollisionSystem.update(colliders, transforms, fixedDt);
            
            // ← 新增：依附同步系统（同步 Hitbox 到主人）
            attachmentSystem.update(attachedComponents, transforms, zTransforms, fixedDt);
            
            attackSystem.update(states, attackStates, transforms, characters, ecs, transforms, hitboxes, lifetimes, attachedComponents, zTransforms, fixedDt);
            collisionSystem.update(hitboxes, hurtboxes, transforms, transforms, zTransforms, damageEvents, ecs, fixedDt);
            
            // ← 新增：炸弹系统（倒计时、弹跳、踢飞、爆炸）
            bombSystem.update(bombs, transforms, zTransforms, states, characters, hitboxes, lifetimes, transforms, deathTags, ecs, fixedDt);
            damageSystem.update(characters, damageEvents, deathTags, states, dashes);
            
            lootSpawnSystem.update(transforms, lootDrops, itemDatas, pickupBoxes, deathTags, ecs);
            deathSystem.update(states, transforms, characters, hurtboxes, lootDrops, inputs, deathTags, ecs, fixedDt);
            
            pickupSystem.update(ecs, evolutions, transforms, transforms, itemDatas, pickupBoxes, magnets);
        }
        
        // ← 【核心改动】清理系统（在渲染之后，帧末执行）
        cleanupSystem.update(ecs, deathTags, lifetimes,
            states, transforms, characters, inputs, hurtboxes,
            hitboxes, attackStates, damageTags,
            lootDrops, itemDatas, pickupBoxes,
            magnets, evolutions, dashes,
            bombs, attachedComponents, colliders,
            TIME_PER_FRAME.asSeconds());
        
        // --- 循环外：纯渲染 ---
        window.clear(COLOR_BACKGROUND);
        renderGrid(window);
        
        // ← 【核心改动】Y-Sorting：按逻辑 Y 坐标排序（影子的 Y）
        std::vector<Entity> renderOrder;
        for (Entity entity : characters.entityList()) {
            if (transforms.has(entity) && states.has(entity)) {
                renderOrder.push_back(entity);
            }
        }
        
        // 按 Y 坐标排序（Y 值越大越晚渲染，画在最前面）
        std::sort(renderOrder.begin(), renderOrder.end(), [&transforms](Entity a, Entity b) {
            return transforms.get(a).position.y < transforms.get(b).position.y;
        });
        
        // 按排序顺序渲染
        for (Entity entity : renderOrder) {
            bool isPlayer = (entity == player);
            const DashComponent* dashPtr = dashes.has(entity) ? &dashes.get(entity) : nullptr;
            const ZTransformComponent* zPtr = zTransforms.has(entity) ? &zTransforms.get(entity) : nullptr;
            

            
            // 渲染实体本体（视觉 Y = 逻辑 Y - Z）
            renderEntity(window, transforms.get(entity), characters.get(entity), states.get(entity), isPlayer, dashPtr, zPtr);
        }
        
        renderHitboxes(window, transforms, hitboxes, zTransforms);
        renderLoot(window, transforms, itemDatas);
        renderBombs(window, transforms, bombs, zTransforms);  // ← 新增：渲染炸弹
        window.display();
        
        // ← 调试输出
        if (debugClock.getElapsedTime().asSeconds() >= 1.0f) {
            const auto& evolution = evolutions.get(player);
            float playerZ = zTransforms.has(player) ? zTransforms.get(player).z : 0.0f;
            std::cout << "Player HP: " << characters.get(player).currentHP
                      << " | Evo Points: " << evolution.evolutionPoints
                      << " | State: " << stateToString(states.get(player).currentState)
                      << " | Z-Height: " << playerZ
                      << " | Loots: " << itemDatas.entityList().size()
                      << " | TimeScale: " << timeScale << "\n";
            debugClock.restart();
        }
    }
    
    return 0;
}

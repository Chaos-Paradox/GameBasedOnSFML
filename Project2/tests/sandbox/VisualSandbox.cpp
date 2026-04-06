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
                  float x, float y) {
    auto player = ecs.create();
    states.add(player, {CharacterState::Idle, CharacterState::Idle, 0.0f});
    transforms.add(player, {{x, y}, {1.0f, 1.0f}, 0.0f, {0.0f, 0.0f}, 1.0f, 0.0f});
    characters.add(player, {"Player", 1, 100, 100, 10, 5, 200.0f, false, 0.0f, 1.0f, 0.0f});
    inputs.add(player, {{0.0f, 0.0f}, ActionIntent::None, 0.0f});
    hurtboxes.add(player, {{-20, -20, 40, 40}, Faction::Player, 1, 0.0f});
    evolutions.add(player, {0, 0});
    
    // 赋予玩家冲刺能力
    dashes.add(player, {
        .dashSpeed = 2000.0f, 
        .dashDuration = 0.1f,  // ← 减半到 0.1 秒
        .iframeDuration = 0.1f,  // ← 减半到 0.1 秒
        .cooldown = 1.0f, 
        .dashTimer = 0.0f, 
        .cooldownTimer = 0.0f,
        .iframeTimer = 0.0f,
        .dashDir = {1.0f, 0.0f},
        .isInvincible = false  // 只保留 isInvincible 用于渲染
    });
    
    // ← 新增：赋予玩家磁吸能力
    magnets.add(player, {
        .magnetRadius = 150.0f,  // 吸收半径 150 像素
        .magnetSpeed = 400.0f    // 吸收速度 400 像素/秒
    });
    
    return player;
}

auto createDummy(ECS& ecs, ComponentStore<StateMachineComponent>& states, ComponentStore<TransformComponent>& transforms,
                 ComponentStore<CharacterComponent>& characters, ComponentStore<HurtboxComponent>& hurtboxes,
                 ComponentStore<LootDropComponent>& lootDrops, float x, float y) {
    auto dummy = ecs.create();
    states.add(dummy, {CharacterState::Idle, CharacterState::Idle, 0.0f});
    transforms.add(dummy, {{x, y}, {1.0f, 1.0f}, 0.0f, {0.0f, 0.0f}, -1.0f, 0.0f});
    characters.add(dummy, {"Dummy", 1, 100, 100, 10, 0, 0.0f, false, 0.0f, -1.0f, 0.0f});
    hurtboxes.add(dummy, {{-20, -20, 40, 40}, Faction::Enemy, 2, 0.0f});
    
    LootDropComponent dummyLoot;
    dummyLoot.lootTable[0] = {1, 1.0f, 1, 1};
    dummyLoot.lootCount = 1;
    dummyLoot.hasDropped = false;
    lootDrops.add(dummy, dummyLoot);
    
    return dummy;
}

// 渲染实体（新增 Dash 参数用于渲染无敌帧特效）
void renderEntity(sf::RenderWindow& window, const TransformComponent& trans, const CharacterComponent& chara,
                  const StateMachineComponent& state, bool isPlayer, const DashComponent* dash = nullptr) {
    sf::RectangleShape rect({ENTITY_SIZE, ENTITY_SIZE});
    rect.setOrigin({ENTITY_SIZE / 2.0f, ENTITY_SIZE / 2.0f});
    rect.setPosition({trans.position.x, trans.position.y});
    
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
                    const ComponentStore<HitboxComponent>& hitboxes) {
    for (Entity entity : hitboxes.entityList()) {
        if (!transforms.has(entity)) continue;
        const auto& transform = transforms.get(entity);
        const auto& hitbox = hitboxes.get(entity);
        float centerX = transform.position.x + hitbox.bounds.x + hitbox.bounds.width / 2.0f;
        float centerY = transform.position.y + hitbox.bounds.y + hitbox.bounds.height / 2.0f;
        sf::RectangleShape rect({hitbox.bounds.width, hitbox.bounds.height});
        rect.setOrigin({hitbox.bounds.width / 2.0f, hitbox.bounds.height / 2.0f});
        rect.setPosition({centerX, centerY});
        rect.setFillColor(COLOR_HITBOX);
        window.draw(rect);
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

    auto player = createPlayer(ecs, states, transforms, characters, inputs, hurtboxes, evolutions, dashes, magnets, 200, 300);
    auto dummy = createDummy(ecs, states, transforms, characters, hurtboxes, lootDrops, 700, 300);
    
    // 🚀 神圣时间常数：锁死 60 Hz 物理运算
    const sf::Time TIME_PER_FRAME = sf::seconds(1.0f / 60.0f);
    sf::Time timeSinceLastUpdate = sf::Time::Zero;
    sf::Clock clock, debugClock;
    
    // 输入缓存变量
    bool lastJPressed = false;
    bool lastSpacePressed = false;
    
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
                if (kp->code == sf::Keyboard::Key::K) {
                    frameStep = true;  // 单帧步进
                    timeScale = 1.0f;  // 临时恢复时间
                }
            }
            
            // ← 右键点击生成假人
            if (const auto* mb = event->getIf<sf::Event::MouseButtonPressed>()) {
                if (mb->button == sf::Mouse::Button::Right) {
                    sf::Vector2f worldPos = window.mapPixelToCoords(mb->position);
                    createDummy(ecs, states, transforms, characters, hurtboxes, lootDrops, worldPos.x, worldPos.y);
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
            
            // 位移执行系统（将冲刺速度化为实质距离）
            movementSystem.update(transforms, itemDatas, fixedDt);
            
            attackSystem.update(states, attackStates, transforms, characters, ecs, transforms, hitboxes, lifetimes, fixedDt);
            collisionSystem.update(hitboxes, hurtboxes, transforms, transforms, damageEvents, ecs, fixedDt);
            damageSystem.update(characters, damageEvents, deathTags, states, dashes);
            
            lootSpawnSystem.update(transforms, lootDrops, itemDatas, pickupBoxes, deathTags, ecs);
            deathSystem.update(states, transforms, characters, hurtboxes, lootDrops, inputs, deathTags, ecs, fixedDt);
            
            pickupSystem.update(ecs, evolutions, transforms, transforms, itemDatas, pickupBoxes, magnets);
            cleanupSystem.update(lifetimes, transforms, hitboxes, magnets, itemDatas, pickupBoxes, damageEvents, ecs, fixedDt);
        }
        
        // --- 循环外：纯渲染 ---
        window.clear(COLOR_BACKGROUND);
        renderGrid(window);
        
        for (Entity entity : characters.entityList()) {
            if (!transforms.has(entity) || !states.has(entity)) continue;
            
            bool isPlayer = (entity == player);
            const DashComponent* dashPtr = dashes.has(entity) ? &dashes.get(entity) : nullptr;
            
            renderEntity(window, transforms.get(entity), characters.get(entity), states.get(entity), isPlayer, dashPtr);
        }
        
        renderHitboxes(window, transforms, hitboxes);
        renderLoot(window, transforms, itemDatas);
        window.display();
        
        // ← 【恢复】调试输出
        if (debugClock.getElapsedTime().asSeconds() >= 1.0f) {
            const auto& evolution = evolutions.get(player);
            std::cout << "Player HP: " << characters.get(player).currentHP
                      << " | Evo Points: " << evolution.evolutionPoints
                      << " | State: " << stateToString(states.get(player).currentState)
                      << " | Loots: " << itemDatas.entityList().size()
                      << " | TimeScale: " << timeScale << "\n";
            debugClock.restart();
        }
    }
    
    return 0;
}

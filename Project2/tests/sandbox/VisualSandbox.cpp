/**
 * @file VisualSandbox.cpp
 * @brief 可视化调试沙盒 - 战斗管线测试（含战利品系统）
 * 
 * 功能：
 * - WASD 移动玩家
 * - J 键触发攻击
 * - 击杀怪物掉落进化点数
 * - 玩家拾取掉落物增加面板数值
 * - 血条显示
 * - 掉落物可视化（绿色圆圈）
 */

#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <iostream>
#include <string>
#include <vector>

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
#include "systems/StateMachineSystem.h"
#include "systems/LocomotionSystem.h"
#include "systems/MovementSystem.h"
#include "systems/AttackSystem.h"
#include "systems/CollisionSystem.h"
#include "systems/DamageSystem.h"
#include "systems/DeathSystem.h"
#include "systems/LootSpawnSystem.h"
#include "systems/PickupSystem.h"
#include "systems/CleanupSystem.h"

constexpr int WINDOW_WIDTH = 1024;
constexpr int WINDOW_HEIGHT = 768;
constexpr float ENTITY_SIZE = 40.0f;
constexpr float DT = 1.0f / 60.0f;

sf::Color COLOR_PLAYER(50, 200, 50);
sf::Color COLOR_ENEMY(200, 50, 50);
sf::Color COLOR_HURT(255, 100, 100);
sf::Color COLOR_DEAD(100, 100, 100);
sf::Color COLOR_HITBOX(255, 255, 0, 128);
sf::Color COLOR_LOOT(50, 255, 50);  // ← 掉落物绿色
sf::Color COLOR_BACKGROUND(30, 30, 30);

std::string stateToString(CharacterState state) {
    switch(state) {
        case CharacterState::Idle: return "IDLE";
        case CharacterState::Move: return "MOVE";
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
                  float x, float y) {
    auto player = ecs.create();
    states.add(player, {CharacterState::Idle, CharacterState::Idle, 0.0f});
    transforms.add(player, {{x, y}, {1.0f, 1.0f}, 0.0f, {0.0f, 0.0f}, 1.0f, 0.0f});
    characters.add(player, {"Player", 1, 100, 100, 10, 5, 200.0f, false, 0.0f, 1.0f, 0.0f});
    inputs.add(player, {{0.0f, 0.0f}, false});
    hurtboxes.add(player, {{-20, -20, 40, 40}, Faction::Player, 1, 0.0f});
    evolutions.add(player, {0, 0});
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
    
    // ← 修复：添加掉落表：100% 掉落 1 个进化点（itemId=1）
    LootDropComponent dummyLoot;
    dummyLoot.lootTable[0] = {1, 1.0f, 1, 1};
    dummyLoot.lootCount = 1;
    dummyLoot.hasDropped = false;
    lootDrops.add(dummy, dummyLoot);
    
    return dummy;
}

// ← 修复 2：添加血条渲染
void renderEntity(sf::RenderWindow& window, const TransformComponent& trans, const CharacterComponent& chara,
                  const StateMachineComponent& state, bool isPlayer) {
    // 绘制实体方块
    sf::RectangleShape rect({ENTITY_SIZE, ENTITY_SIZE});
    rect.setOrigin({ENTITY_SIZE / 2.0f, ENTITY_SIZE / 2.0f});
    rect.setPosition({trans.position.x, trans.position.y});
    if (state.currentState == CharacterState::Dead) rect.setFillColor(COLOR_DEAD);
    else if (state.currentState == CharacterState::Hurt) rect.setFillColor(COLOR_HURT);
    else if (isPlayer) rect.setFillColor(COLOR_PLAYER);
    else rect.setFillColor(COLOR_ENEMY);
    window.draw(rect);
    
    // ← 绘制血条（在实体上方）
    float hpBarWidth = 40.0f;
    float hpBarHeight = 5.0f;
    float hpBarY = trans.position.y - ENTITY_SIZE / 2.0f - 8.0f;  // 实体上方 8 像素
    
    // 血条底色（红色）
    sf::RectangleShape hpBarBg({hpBarWidth, hpBarHeight});
    hpBarBg.setOrigin({hpBarWidth / 2.0f, hpBarHeight / 2.0f});
    hpBarBg.setPosition({trans.position.x, hpBarY});
    hpBarBg.setFillColor(sf::Color(200, 0, 0));
    window.draw(hpBarBg);
    
    // 血条当前值（绿色）
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

// ← 修复 3：渲染掉落物（绿色小圆圈）
void renderLoot(sf::RenderWindow& window, const ComponentStore<TransformComponent>& transforms,
                const ComponentStore<ItemDataComponent>& itemDatas) {
    for (Entity loot : itemDatas.entityList()) {
        if (!transforms.has(loot)) continue;
        const auto& transform = transforms.get(loot);
        
        // 绘制绿色圆圈代表掉落物
        sf::CircleShape lootCircle(8.0f);  // 半径 8 像素
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
    std::cout << "=== Project2 Loot Pipeline Sandbox ===\n";
    std::cout << "Controls: WASD to move, J to attack, Kill dummy to get evolution points!\n";
    std::cout << "Visual: Red bar = HP, Green circle = Loot\n\n";
    
    sf::RenderWindow window(sf::VideoMode({WINDOW_WIDTH, WINDOW_HEIGHT}), "Project2 Loot & Pickup Pipeline");
    window.setFramerateLimit(60);
    
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
    ComponentStore<DeathTag> deathTags;
    
    // ← 战利品系统组件
    ComponentStore<LootDropComponent> lootDrops;
    ComponentStore<ItemDataComponent> itemDatas;
    ComponentStore<PickupBoxComponent> pickupBoxes;
    ComponentStore<EvolutionComponent> evolutions;
    
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
    CleanupSystem cleanupSystem;
    
    // 创建玩家
    auto player = createPlayer(ecs, states, transforms, characters, inputs, hurtboxes, evolutions, 200, 300);
    
    // 创建假人（带掉落表）
    auto dummy = createDummy(ecs, states, transforms, characters, hurtboxes, lootDrops, 700, 300);
    
    std::cout << "Player created at (200, 300)\n";
    std::cout << "Dummy created at (700, 300) with 100% drop rate\n";
    std::cout << "Press J to attack, walk over loot to pickup\n\n";
    
    sf::Clock clock, debugClock;
    
    while (window.isOpen()) {
        float dt = clock.restart().asSeconds();
        
        // 事件处理
        while (const auto event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) window.close();
            if (const auto* kp = event->getIf<sf::Event::KeyPressed>()) {
                if (kp->code == sf::Keyboard::Key::Escape) window.close();
            }
        }
        
        // 玩家输入
        inputs.get(player).moveDir = getInputFromKeyboard();
        static bool lastJPressed = false;
        bool currentJPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::J);
        inputs.get(player).attackPressed = currentJPressed && !lastJPressed;
        lastJPressed = currentJPressed;
        
        // ========== 战斗管线 ==========
        stateSystem.update(states, attackStates, inputs, damageTags, ecs, dt);
        damageSystem.update(characters, damageTags, deathTags);
        
        // 4. LootSpawnSystem ⭐【新增】(抢在尸体被清空前，生成掉落物)
        lootSpawnSystem.update(transforms, lootDrops, itemDatas, pickupBoxes, deathTags, ecs);
        
        // 5. DeathSystem ← 修复：传入 ecs 对象
        deathSystem.update(states, deathTags, ecs, dt);
        
        locomotionSystem.update(states, transforms, characters, inputs, dt);
        movementSystem.update(transforms, dt);
        attackSystem.update(states, attackStates, transforms, characters, ecs, transforms, hitboxes, lifetimes, dt);
        collisionSystem.update(hitboxes, hurtboxes, transforms, transforms, damageTags, dt);
        
        // 10. PickupSystem ⭐【新增】(拾取判定：吃掉落物，加点数，销毁掉落物)
        pickupSystem.update(evolutions, transforms, transforms, itemDatas, pickupBoxes);
        
        cleanupSystem.update(lifetimes, transforms, hitboxes, dt);
        
        // 渲染
        window.clear(COLOR_BACKGROUND);
        renderGrid(window);
        renderEntity(window, transforms.get(player), characters.get(player), states.get(player), true);
        renderEntity(window, transforms.get(dummy), characters.get(dummy), states.get(dummy), false);
        renderHitboxes(window, transforms, hitboxes);
        renderLoot(window, transforms, itemDatas);  // ← 修复 3：渲染掉落物
        window.display();
        
        // 调试输出（低频）
        if (debugClock.getElapsedTime().asSeconds() >= 1.0f) {
            const auto& evolution = evolutions.get(player);
            std::cout << "Player HP: " << characters.get(player).currentHP
                      << " | Evo Points: " << evolution.evolutionPoints
                      << " | State: " << stateToString(states.get(player).currentState)
                      << " | Loots: " << itemDatas.entityList().size() << "\n";
            debugClock.restart();
        }
    }
    
    std::cout << "\nSandbox closed.\n";
    return 0;
}

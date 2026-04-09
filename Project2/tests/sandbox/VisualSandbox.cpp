/**
 * @file VisualSandbox.cpp
 * @brief 可视化调试沙盒 - 战斗管线测试（含冲刺与战利品系统）
 * 
 * 输入系统：使用 InputManager 进行平台无关的按键绑定
 */

#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <iostream>
#include <string>
#include <vector>
#include <cmath>
#include <algorithm>
#include <unordered_map>

// ========== 核心输入管理器（平台无关层）==========
#include "core/InputManager.h"

// ========== 平台适配器：引擎键 <-> SFML 键双向翻译 ==========

/**
 * @brief 将引擎键翻译为 SFML 键（用于实时状态查询）
 */
sf::Keyboard::Key toSFMLKey(EngineKey eKey) {
    switch(eKey) {
        case EngineKey::W: return sf::Keyboard::Key::W;
        case EngineKey::A: return sf::Keyboard::Key::A;
        case EngineKey::S: return sf::Keyboard::Key::S;
        case EngineKey::D: return sf::Keyboard::Key::D;
        case EngineKey::Space: return sf::Keyboard::Key::Space;
        case EngineKey::Escape: return sf::Keyboard::Key::Escape;
        case EngineKey::J: return sf::Keyboard::Key::J;
        case EngineKey::K: return sf::Keyboard::Key::K;
        case EngineKey::G: return sf::Keyboard::Key::G;
        case EngineKey::T: return sf::Keyboard::Key::T;
        case EngineKey::H: return sf::Keyboard::Key::H;
        case EngineKey::L: return sf::Keyboard::Key::L;
        case EngineKey::Q: return sf::Keyboard::Key::Q;
        case EngineKey::R: return sf::Keyboard::Key::R;
        case EngineKey::E: return sf::Keyboard::Key::E;
        case EngineKey::F: return sf::Keyboard::Key::F;
        case EngineKey::P: return sf::Keyboard::Key::P;
        case EngineKey::O: return sf::Keyboard::Key::O;
        case EngineKey::I: return sf::Keyboard::Key::I;
        case EngineKey::U: return sf::Keyboard::Key::U;
        case EngineKey::Y: return sf::Keyboard::Key::Y;
        case EngineKey::X: return sf::Keyboard::Key::X;
        case EngineKey::F1: return sf::Keyboard::Key::F1;
        case EngineKey::F2: return sf::Keyboard::Key::F2;
        case EngineKey::F3: return sf::Keyboard::Key::F3;
        case EngineKey::F4: return sf::Keyboard::Key::F4;
        case EngineKey::F5: return sf::Keyboard::Key::F5;
        case EngineKey::F6: return sf::Keyboard::Key::F6;
        case EngineKey::F7: return sf::Keyboard::Key::F7;
        case EngineKey::F8: return sf::Keyboard::Key::F8;
        case EngineKey::F9: return sf::Keyboard::Key::F9;
        case EngineKey::F10: return sf::Keyboard::Key::F10;
        case EngineKey::F11: return sf::Keyboard::Key::F11;
        case EngineKey::F12: return sf::Keyboard::Key::F12;
        case EngineKey::Enter: return sf::Keyboard::Key::Enter;
        case EngineKey::LeftShift: return sf::Keyboard::Key::LShift;
        case EngineKey::RightShift: return sf::Keyboard::Key::RShift;
        case EngineKey::LeftCtrl: return sf::Keyboard::Key::LControl;
        case EngineKey::RightCtrl: return sf::Keyboard::Key::RControl;
        case EngineKey::LeftAlt: return sf::Keyboard::Key::LAlt;
        case EngineKey::RightAlt: return sf::Keyboard::Key::RAlt;
        case EngineKey::Z: return sf::Keyboard::Key::Z;
        case EngineKey::C: return sf::Keyboard::Key::C;
        case EngineKey::V: return sf::Keyboard::Key::V;
        case EngineKey::B: return sf::Keyboard::Key::B;
        case EngineKey::N: return sf::Keyboard::Key::N;
        case EngineKey::M: return sf::Keyboard::Key::M;
        case EngineKey::Comma: return sf::Keyboard::Key::Comma;
        case EngineKey::Period: return sf::Keyboard::Key::Period;
        case EngineKey::Slash: return sf::Keyboard::Key::Slash;
        case EngineKey::SemiColon: return sf::Keyboard::Key::Semicolon;
        case EngineKey::Quote: return sf::Keyboard::Key::Apostrophe;
        case EngineKey::LeftBracket: return sf::Keyboard::Key::LBracket;
        case EngineKey::RightBracket: return sf::Keyboard::Key::RBracket;
        case EngineKey::Backslash: return sf::Keyboard::Key::Backslash;
        case EngineKey::Minus: return sf::Keyboard::Key::Hyphen;
        case EngineKey::Equal: return sf::Keyboard::Key::Equal;
        case EngineKey::Backquote: return sf::Keyboard::Key::Grave;
        case EngineKey::Delete: return sf::Keyboard::Key::Delete;
        case EngineKey::Insert: return sf::Keyboard::Key::Insert;
        case EngineKey::Home: return sf::Keyboard::Key::Home;
        case EngineKey::End: return sf::Keyboard::Key::End;
        case EngineKey::PageUp: return sf::Keyboard::Key::PageUp;
        case EngineKey::PageDown: return sf::Keyboard::Key::PageDown;
        case EngineKey::Up: return sf::Keyboard::Key::Up;
        case EngineKey::Down: return sf::Keyboard::Key::Down;
        case EngineKey::Left: return sf::Keyboard::Key::Left;
        case EngineKey::Right: return sf::Keyboard::Key::Right;
        case EngineKey::Num0: return sf::Keyboard::Key::Numpad0;
        case EngineKey::Num1: return sf::Keyboard::Key::Numpad1;
        case EngineKey::Num2: return sf::Keyboard::Key::Numpad2;
        case EngineKey::Num3: return sf::Keyboard::Key::Numpad3;
        case EngineKey::Num4: return sf::Keyboard::Key::Numpad4;
        case EngineKey::Num5: return sf::Keyboard::Key::Numpad5;
        case EngineKey::Num6: return sf::Keyboard::Key::Numpad6;
        case EngineKey::Num7: return sf::Keyboard::Key::Numpad7;
        case EngineKey::Num8: return sf::Keyboard::Key::Numpad8;
        case EngineKey::Num9: return sf::Keyboard::Key::Numpad9;
        // 鼠标按键（通过特殊值标识，实际查询用 isMouseButtonPressed）
        case EngineKey::MouseLeft: return sf::Keyboard::Key::Unknown;
        case EngineKey::MouseRight: return sf::Keyboard::Key::Unknown;
        case EngineKey::MouseMiddle: return sf::Keyboard::Key::Unknown;
        case EngineKey::MouseX1: return sf::Keyboard::Key::Unknown;
        case EngineKey::MouseX2: return sf::Keyboard::Key::Unknown;
        default: return sf::Keyboard::Key::Unknown;
    }
}

/**
 * @brief 将 SFML 键翻译为引擎键（用于捕获改键输入）
 */
EngineKey toEngineKey(sf::Keyboard::Key sfKey) {
    switch(sfKey) {
        case sf::Keyboard::Key::W: return EngineKey::W;
        case sf::Keyboard::Key::A: return EngineKey::A;
        case sf::Keyboard::Key::S: return EngineKey::S;
        case sf::Keyboard::Key::D: return EngineKey::D;
        case sf::Keyboard::Key::Space: return EngineKey::Space;
        case sf::Keyboard::Key::Escape: return EngineKey::Escape;
        case sf::Keyboard::Key::J: return EngineKey::J;
        case sf::Keyboard::Key::K: return EngineKey::K;
        case sf::Keyboard::Key::G: return EngineKey::G;
        case sf::Keyboard::Key::T: return EngineKey::T;
        case sf::Keyboard::Key::H: return EngineKey::H;
        case sf::Keyboard::Key::L: return EngineKey::L;
        case sf::Keyboard::Key::Q: return EngineKey::Q;
        case sf::Keyboard::Key::R: return EngineKey::R;
        case sf::Keyboard::Key::E: return EngineKey::E;
        case sf::Keyboard::Key::F: return EngineKey::F;
        case sf::Keyboard::Key::P: return EngineKey::P;
        case sf::Keyboard::Key::O: return EngineKey::O;
        case sf::Keyboard::Key::I: return EngineKey::I;
        case sf::Keyboard::Key::U: return EngineKey::U;
        case sf::Keyboard::Key::Y: return EngineKey::Y;
        case sf::Keyboard::Key::F1: return EngineKey::F1;
        case sf::Keyboard::Key::F2: return EngineKey::F2;
        case sf::Keyboard::Key::F3: return EngineKey::F3;
        case sf::Keyboard::Key::F4: return EngineKey::F4;
        case sf::Keyboard::Key::F5: return EngineKey::F5;
        case sf::Keyboard::Key::F6: return EngineKey::F6;
        case sf::Keyboard::Key::F7: return EngineKey::F7;
        case sf::Keyboard::Key::F8: return EngineKey::F8;
        case sf::Keyboard::Key::F9: return EngineKey::F9;
        case sf::Keyboard::Key::F10: return EngineKey::F10;
        case sf::Keyboard::Key::F11: return EngineKey::F11;
        case sf::Keyboard::Key::F12: return EngineKey::F12;
        case sf::Keyboard::Key::Enter: return EngineKey::Enter;
        case sf::Keyboard::Key::LShift: return EngineKey::LeftShift;
        case sf::Keyboard::Key::RShift: return EngineKey::RightShift;
        case sf::Keyboard::Key::LControl: return EngineKey::LeftCtrl;
        case sf::Keyboard::Key::RControl: return EngineKey::RightCtrl;
        case sf::Keyboard::Key::LAlt: return EngineKey::LeftAlt;
        case sf::Keyboard::Key::RAlt: return EngineKey::RightAlt;
        case sf::Keyboard::Key::Z: return EngineKey::Z;
        case sf::Keyboard::Key::X: return EngineKey::X;
        case sf::Keyboard::Key::C: return EngineKey::C;
        case sf::Keyboard::Key::V: return EngineKey::V;
        case sf::Keyboard::Key::B: return EngineKey::B;
        case sf::Keyboard::Key::N: return EngineKey::N;
        case sf::Keyboard::Key::M: return EngineKey::M;
        case sf::Keyboard::Key::Comma: return EngineKey::Comma;
        case sf::Keyboard::Key::Period: return EngineKey::Period;
        case sf::Keyboard::Key::Slash: return EngineKey::Slash;
        case sf::Keyboard::Key::Semicolon: return EngineKey::SemiColon;
        case sf::Keyboard::Key::Apostrophe: return EngineKey::Quote;
        case sf::Keyboard::Key::LBracket: return EngineKey::LeftBracket;
        case sf::Keyboard::Key::RBracket: return EngineKey::RightBracket;
        case sf::Keyboard::Key::Backslash: return EngineKey::Backslash;
        case sf::Keyboard::Key::Hyphen: return EngineKey::Minus;
        case sf::Keyboard::Key::Equal: return EngineKey::Equal;
        case sf::Keyboard::Key::Grave: return EngineKey::Backquote;
        case sf::Keyboard::Key::Delete: return EngineKey::Delete;
        case sf::Keyboard::Key::Insert: return EngineKey::Insert;
        case sf::Keyboard::Key::Home: return EngineKey::Home;
        case sf::Keyboard::Key::End: return EngineKey::End;
        case sf::Keyboard::Key::PageUp: return EngineKey::PageUp;
        case sf::Keyboard::Key::PageDown: return EngineKey::PageDown;
        case sf::Keyboard::Key::Up: return EngineKey::Up;
        case sf::Keyboard::Key::Down: return EngineKey::Down;
        case sf::Keyboard::Key::Left: return EngineKey::Left;
        case sf::Keyboard::Key::Right: return EngineKey::Right;
        case sf::Keyboard::Key::Numpad0: return EngineKey::Num0;
        case sf::Keyboard::Key::Numpad1: return EngineKey::Num1;
        case sf::Keyboard::Key::Numpad2: return EngineKey::Num2;
        case sf::Keyboard::Key::Numpad3: return EngineKey::Num3;
        case sf::Keyboard::Key::Numpad4: return EngineKey::Num4;
        case sf::Keyboard::Key::Numpad5: return EngineKey::Num5;
        case sf::Keyboard::Key::Numpad6: return EngineKey::Num6;
        case sf::Keyboard::Key::Numpad7: return EngineKey::Num7;
        case sf::Keyboard::Key::Numpad8: return EngineKey::Num8;
        case sf::Keyboard::Key::Numpad9: return EngineKey::Num9;
        default: return EngineKey::Unknown;
    }
}

/**
 * @brief 将 SFML 鼠标键翻译为引擎键
 */
EngineKey toEngineKey(sf::Mouse::Button mouseButton) {
    switch(mouseButton) {
        case sf::Mouse::Button::Left: return EngineKey::MouseLeft;
        case sf::Mouse::Button::Right: return EngineKey::MouseRight;
        case sf::Mouse::Button::Middle: return EngineKey::MouseMiddle;
        case sf::Mouse::Button::Extra1: return EngineKey::MouseX1;
        case sf::Mouse::Button::Extra2: return EngineKey::MouseX2;
        default: return EngineKey::Unknown;
    }
}

/**
 * @brief 基于 Action 的实时查询函数（供沙盒使用，支持键盘 + 鼠标）
 */
bool isActionPressed(const InputManager& input, GameAction action) {
    EngineKey eKey = input.getMappedKey(action);
    
    // 检查是否为鼠标按键
    if (eKey == EngineKey::MouseLeft) return sf::Mouse::isButtonPressed(sf::Mouse::Button::Left);
    if (eKey == EngineKey::MouseRight) return sf::Mouse::isButtonPressed(sf::Mouse::Button::Right);
    if (eKey == EngineKey::MouseMiddle) return sf::Mouse::isButtonPressed(sf::Mouse::Button::Middle);
    if (eKey == EngineKey::MouseX1) return sf::Mouse::isButtonPressed(sf::Mouse::Button::Extra1);
    if (eKey == EngineKey::MouseX2) return sf::Mouse::isButtonPressed(sf::Mouse::Button::Extra2);
    
    // 否则按键盘处理
    return sf::Keyboard::isKeyPressed(toSFMLKey(eKey));
}

// ========== ECS 核心与组件 ==========
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
#include "systems/DamageTextSpawnerSystem.h"
#include "systems/DamageTextRenderSystem.h"

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

/**
 * @brief 根据按键绑定获取移动输入（已归一化）
 */
Vec2 getMappedInput(const InputManager& input) {
    Vec2 dir{0.0f, 0.0f};
    if (isActionPressed(input, GameAction::Up)) dir.y -= 1.0f;
    if (isActionPressed(input, GameAction::Down)) dir.y += 1.0f;
    if (isActionPressed(input, GameAction::Left)) dir.x -= 1.0f;
    if (isActionPressed(input, GameAction::Right)) dir.x += 1.0f;
    
    // 向量归一化，防止斜向移动速度变成 1.414 倍
    if (dir.x != 0.0f || dir.y != 0.0f) {
        float length = std::sqrt(dir.x * dir.x + dir.y * dir.y);
        if (length > 0.0f) {
            dir.x /= length;
            dir.y /= length;
        }
    }
    
    return dir;
}

auto createPlayer(ECS& ecs, ComponentStore<StateMachineComponent>& states, ComponentStore<TransformComponent>& transforms,
                  ComponentStore<CharacterComponent>& characters, ComponentStore<InputCommand>& inputs,
                  ComponentStore<HurtboxComponent>& hurtboxes, ComponentStore<EvolutionComponent>& evolutions,
                  ComponentStore<DashComponent>& dashes, ComponentStore<MagnetComponent>& magnets,
                  ComponentStore<ZTransformComponent>& zTransforms,
                  ComponentStore<ColliderComponent>& colliders,
                  float x, float y) {
    auto player = ecs.create();
    states.add(player, {CharacterState::Idle, CharacterState::Idle, 0.0f});
    transforms.add(player, {{x, y}, {1.0f, 1.0f}, 0.0f, {0.0f, 0.0f}, 1.0f, 0.0f});
    characters.add(player, {"Player", 1, 100, 100, 10, 5, 200.0f, false, 0.0f, 1.0f, 0.0f});
    inputs.add(player, {{0.0f, 0.0f}, ActionIntent::None, 0.0f});
    hurtboxes.add(player, {20.0f, {0.0f, 0.0f}, Faction::Player, 1, 0.0f});
    evolutions.add(player, {0, 0});
    
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
    
    magnets.add(player, {
        .magnetRadius = 150.0f,
        .magnetSpeed = 400.0f
    });
    
    zTransforms.add(player, {
        .z = 0.0f,
        .vz = 0.0f,
        .gravity = -2000.0f,
        .height = 40.0f
    });
    
    colliders.add(player, {
        .radius = 20.0f,
        .isStatic = false,
        .mass = 100.0f
    });
    
    return player;
}

auto createDummy(ECS& ecs, ComponentStore<StateMachineComponent>& states, ComponentStore<TransformComponent>& transforms,
                 ComponentStore<CharacterComponent>& characters, ComponentStore<HurtboxComponent>& hurtboxes,
                 ComponentStore<LootDropComponent>& lootDrops,
                 ComponentStore<ColliderComponent>& colliders,
                 float x, float y) {
    auto dummy = ecs.create();
    states.add(dummy, {CharacterState::Idle, CharacterState::Idle, 0.0f});
    transforms.add(dummy, {{x, y}, {1.0f, 1.0f}, 0.0f, {0.0f, 0.0f}, -1.0f, 0.0f});
    characters.add(dummy, {"Dummy", 1, 100, 100, 10, 0, 0.0f, false, 0.0f, -1.0f, 0.0f});
    hurtboxes.add(dummy, {20.0f, {0.0f, 0.0f}, Faction::Enemy, 2, 0.0f});
    
    LootDropComponent dummyLoot;
    dummyLoot.lootTable[0] = {1, 1.0f, 1, 1};
    dummyLoot.lootCount = 1;
    dummyLoot.hasDropped = false;
    lootDrops.add(dummy, dummyLoot);
    
    colliders.add(dummy, {
        .radius = 20.0f,
        .isStatic = false,
        .mass = 100.0f
    });
    
    return dummy;
}

void renderEntity(sf::RenderWindow& window, const TransformComponent& trans, const CharacterComponent& chara,
                  const StateMachineComponent& state, bool isPlayer, const DashComponent* dash = nullptr,
                  const ZTransformComponent* zComp = nullptr) {
    sf::CircleShape shadow(30.0f);
    shadow.setOrigin({30.0f, 15.0f});
    shadow.setScale({1.0f, 0.5f});
    shadow.setFillColor(sf::Color(0, 0, 0, 100));
    shadow.setPosition({trans.position.x, trans.position.y});
    window.draw(shadow);
    
    sf::RectangleShape rect({ENTITY_SIZE, ENTITY_SIZE});
    rect.setOrigin({ENTITY_SIZE / 2.0f, ENTITY_SIZE / 2.0f});
    
    float renderX = trans.position.x;
    float renderY = trans.position.y;
    if (zComp && zComp->z > 0.0f) {
        renderY -= zComp->z;
    }
    
    rect.setPosition({renderX, renderY});
    
    sf::Color entityColor = isPlayer ? COLOR_PLAYER : COLOR_ENEMY;

    if (state.currentState == CharacterState::Dead) {
        entityColor = COLOR_DEAD;
    } else if (state.currentState == CharacterState::Hurt) {
        entityColor = COLOR_HURT;
    } else if (state.currentState == CharacterState::Dash && dash != nullptr) {
        if (dash->isInvincible) {
            entityColor = sf::Color(0, 255, 255, 180);
        } else {
            entityColor = sf::Color(isPlayer ? 50 : 200, 100, 100, 200);
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
                    const ComponentStore<ZTransformComponent>& zTransforms) {
    for (Entity entity : hitboxes.entityList()) {
        if (!transforms.has(entity)) continue;
        const auto& transform = transforms.get(entity);
        const auto& hitbox = hitboxes.get(entity);
        
        float z = zTransforms.has(entity) ? zTransforms.get(entity).z : 0.0f;
        
        float centerX = transform.position.x + hitbox.offset.x;
        float centerY = transform.position.y + hitbox.offset.y - z;
        
        sf::CircleShape circle(hitbox.radius);
        circle.setOrigin({hitbox.radius, hitbox.radius});
        circle.setPosition({centerX, centerY});
        circle.setFillColor(sf::Color(255, 255, 0, 128));
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

void renderBombs(sf::RenderWindow& window, const ComponentStore<TransformComponent>& transforms,
                 const ComponentStore<BombComponent>& bombs,
                 const ComponentStore<ZTransformComponent>& zTransforms) {
    for (Entity entity : bombs.entityList()) {
        if (!transforms.has(entity)) continue;
        const auto& transform = transforms.get(entity);
        const auto& bomb = bombs.get(entity);
        
        float z = zTransforms.has(entity) ? zTransforms.get(entity).z : 0.0f;
        
        float centerX = transform.position.x;
        float centerY = transform.position.y - z;
        
        sf::CircleShape bombCircle(15.0f);
        bombCircle.setOrigin({15.0f, 15.0f});
        bombCircle.setPosition({centerX, centerY});
        bombCircle.setFillColor(sf::Color(0, 0, 0));
        
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
    window.setFramerateLimit(144);
    
    float timeScale = 1.0f;
    bool frameStep = false;
    
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
    ComponentStore<ZTransformComponent> zTransforms;
    ComponentStore<ColliderComponent> colliders;
    ComponentStore<AttachedComponent> attachedComponents;
    ComponentStore<BombComponent> bombs;
    ComponentStore<DamageTextComponent> damageTexts;

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
    PhysicalCollisionSystem physicalCollisionSystem;
    AttachmentSystem attachmentSystem;
    BombSystem bombSystem;
    DamageTextSpawnerSystem damageTextSpawnerSystem;
    DamageTextRenderSystem damageTextRenderSystem;
    
    sf::Font font;
    if (!font.openFromFile("/System/Library/Fonts/Supplemental/Arial Unicode.ttf")) {
        font.openFromFile("/System/Library/Fonts/Helvetica.ttc");
    }

    auto player = createPlayer(ecs, states, transforms, characters, inputs, hurtboxes, evolutions, dashes, magnets, zTransforms, colliders, 200, 300);
    auto dummy = createDummy(ecs, states, transforms, characters, hurtboxes, lootDrops, colliders, 700, 300);
    
    const sf::Time TIME_PER_FRAME = sf::seconds(1.0f / 60.0f);
    sf::Time timeSinceLastUpdate = sf::Time::Zero;
    sf::Clock clock, debugClock;
    
    // ========== 输入管理器初始化 ==========
    InputManager inputManager;
    
    // 输入缓存变量
    bool lastAttackPressed = false;
    bool lastDashPressed = false;
    bool lastBombPressed = false;
    bool lastHelpPressed = false;
    
    // UI 帮助面板开关
    bool showHelpUI = false;
    
    // ========== 改键状态机 ==========
    bool isRebinding = false;
    GameAction actionToRebind = GameAction::Attack;
    
    // ========== 菜单导航状态 ==========
    // 可供改键的动作列表（决定菜单显示的顺序）
    std::vector<GameAction> bindableActions = {
        GameAction::Up, GameAction::Down, GameAction::Left, GameAction::Right,
        GameAction::Attack, GameAction::Dash, GameAction::DropBomb,
        GameAction::Jump, GameAction::Pause, GameAction::FrameStep,
        GameAction::SpawnDummy
    };
    
    // 用于 UI 显示的动作名称映射
    std::unordered_map<GameAction, std::string> actionNames = {
        {GameAction::Up, "Move Up"},
        {GameAction::Down, "Move Down"},
        {GameAction::Left, "Move Left"},
        {GameAction::Right, "Move Right"},
        {GameAction::Attack, "Attack"},
        {GameAction::Dash, "Dash / Kick"},
        {GameAction::DropBomb, "Drop Bomb"},
        {GameAction::Jump, "Jump"},
        {GameAction::Pause, "Pause / Resume"},
        {GameAction::FrameStep, "Frame Step"},
        {GameAction::SpawnDummy, "Spawn Dummy (Mouse)"}
    };
    
    int selectedMenuIndex = 0; // 当前选中的菜单行
    
    // 复活倒计时
    float respawnTimer = 0.0f;
    
    while (window.isOpen()) {
        sf::Time dtTime = clock.restart();
        timeSinceLastUpdate += dtTime;
        
        while (const auto event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) window.close();
            
            // ========== 改键拦截逻辑（最高优先级）==========
            // 支持键盘改键
            if (isRebinding && event->is<sf::Event::KeyPressed>()) {
                const auto* kp = event->getIf<sf::Event::KeyPressed>();
                EngineKey pressedKey = toEngineKey(kp->code);
                
                if (pressedKey != EngineKey::Unknown && pressedKey != EngineKey::Escape) {
                    // ========== 按键冲突检测 ==========
                    bool hasConflict = false;
                    GameAction conflictingAction = GameAction::Up; // dummy init
                    
                    for (const auto& [action, key] : inputManager.bindings) {
                        if (key == pressedKey && action != actionToRebind) {
                            hasConflict = true;
                            conflictingAction = action;
                            break;
                        }
                    }
                    
                    if (hasConflict) {
                        // 检测到冲突：先解除旧绑定，再保存新绑定
                        std::cout << "[Rebind] ⚠️  Conflict! " << inputManager.keyToString(pressedKey)
                                  << " is already bound to " << inputManager.actionToString(conflictingAction)
                                  << ". Clearing old binding." << std::endl;
                    }
                    
                    // 保存新绑定（会自动覆盖冲突）
                    inputManager.bindings[actionToRebind] = pressedKey;
                    inputManager.saveConfig();
                    std::cout << "[Rebind] ✅ " << inputManager.actionToString(actionToRebind) 
                              << " -> " << inputManager.keyToString(pressedKey) << std::endl;
                }
                
                isRebinding = false;
                continue; // 绝对拦截！防止按键渗透到游戏逻辑
            }
            else if (isRebinding && event->is<sf::Event::MouseButtonPressed>()) {
                const auto* mb = event->getIf<sf::Event::MouseButtonPressed>();
                EngineKey pressedKey = toEngineKey(mb->button);
                
                if (pressedKey != EngineKey::Unknown) {
                    // 按键冲突检测
                    bool hasConflict = false;
                    GameAction conflictingAction = GameAction::Up;
                    
                    for (const auto& [action, key] : inputManager.bindings) {
                        if (key == pressedKey && action != actionToRebind) {
                            hasConflict = true;
                            conflictingAction = action;
                            break;
                        }
                    }
                    
                    if (hasConflict) {
                        std::cout << "[Rebind] ⚠️  Conflict! " << inputManager.keyToString(pressedKey)
                                  << " is already bound to " << inputManager.actionToString(conflictingAction)
                                  << ". Clearing old binding." << std::endl;
                    }
                    
                    inputManager.bindings[actionToRebind] = pressedKey;
                    inputManager.saveConfig();
                    std::cout << "[Rebind] ✅ " << inputManager.actionToString(actionToRebind) 
                              << " -> " << inputManager.keyToString(pressedKey) << std::endl;
                }
                
                isRebinding = false;
                continue;
            }
            
            // ========== 菜单导航拦截（UI 打开时）==========
            if (showHelpUI && !isRebinding && event->is<sf::Event::KeyPressed>()) {
                const auto* kp = event->getIf<sf::Event::KeyPressed>();
                
                // 上下选择
                if (kp->code == sf::Keyboard::Key::Up || kp->code == sf::Keyboard::Key::W) {
                    selectedMenuIndex = (selectedMenuIndex - 1 + static_cast<int>(bindableActions.size())) % static_cast<int>(bindableActions.size());
                    continue; // 拦截，防止渗透到游戏逻辑
                }
                if (kp->code == sf::Keyboard::Key::Down || kp->code == sf::Keyboard::Key::S) {
                    selectedMenuIndex = (selectedMenuIndex + 1) % static_cast<int>(bindableActions.size());
                    continue; // 拦截
                }
                // 回车确认改键
                if (kp->code == sf::Keyboard::Key::Enter) {
                    isRebinding = true;
                    actionToRebind = bindableActions[selectedMenuIndex];
                    std::cout << "[Rebind] Selecting: " << actionNames[actionToRebind] << "...\n";
                    continue; // 拦截
                }
            }
            
            // 普通按键处理（UI 关闭或菜单未激活时）
            if (const auto* kp = event->getIf<sf::Event::KeyPressed>()) {
                if (kp->code == sf::Keyboard::Key::Escape) window.close();
            }
            
            // 暂停和帧步进（使用输入映射系统，支持动态改键）
            bool currentPausePressed = isActionPressed(inputManager, GameAction::Pause);
            static bool lastPausePressed = false;
            if (currentPausePressed && !lastPausePressed) {
                timeScale = (timeScale == 0.0f) ? 1.0f : 0.0f;
                std::cout << "[TimeScale] " << (timeScale == 0.0f ? "Paused" : "Running") << "\n";
            }
            lastPausePressed = currentPausePressed;
            
            bool currentFrameStepPressed = isActionPressed(inputManager, GameAction::FrameStep);
            static bool lastFrameStepPressed = false;
            if (currentFrameStepPressed && !lastFrameStepPressed) {
                frameStep = true;
                timeScale = 1.0f;
            }
            lastFrameStepPressed = currentFrameStepPressed;
            
            // 生成假人输入（使用输入映射系统，支持动态改键）
            bool spawnDummyPressed = isActionPressed(inputManager, GameAction::SpawnDummy);
            static bool lastSpawnDummyPressed = false;
            if (spawnDummyPressed && !lastSpawnDummyPressed) {
                sf::Vector2i mousePos = sf::Mouse::getPosition(window);
                sf::Vector2f worldPos = window.mapPixelToCoords(mousePos);
                createDummy(ecs, states, transforms, characters, hurtboxes, lootDrops, colliders, worldPos.x, worldPos.y);
                std::cout << "Dummy created at (" << worldPos.x << ", " << worldPos.y << ")\n";
            }
            lastSpawnDummyPressed = spawnDummyPressed;
        }
        
        // --- 循环外：抓取瞬时输入（使用输入映射系统）---
        
        // UI 帮助面板切换
        bool currentHelpPressed = isActionPressed(inputManager, GameAction::ToggleHelp);
        if (currentHelpPressed && !lastHelpPressed) {
            showHelpUI = !showHelpUI;
            std::cout << "[UI] Help overlay: " << (showHelpUI ? "ON" : "OFF") << "\n";
        }
        lastHelpPressed = currentHelpPressed;
        
        // 抓取移动输入（已归一化）
        inputs.get(player).moveDir = getMappedInput(inputManager);
        
        // 攻击指令
        bool currentAttackPressed = isActionPressed(inputManager, GameAction::Attack);
        if (currentAttackPressed && !lastAttackPressed) {
            inputs.get(player).pendingIntent = ActionIntent::Attack;
            inputs.get(player).intentTimer = 0.2f;
            std::cout << "[Input] 🗡️ Attack pressed!\n";
        }
        lastAttackPressed = currentAttackPressed;

        // 冲刺指令（覆盖攻击指令）
        bool currentDashPressed = isActionPressed(inputManager, GameAction::Dash);
        if (currentDashPressed && !lastDashPressed) {
            inputs.get(player).pendingIntent = ActionIntent::Dash;
            inputs.get(player).intentTimer = 0.2f;
            std::cout << "[Input] 💨 Dash pressed!\n";
        }
        lastDashPressed = currentDashPressed;
        
        // 跳跃输入（使用输入映射系统，支持动态改键）
        bool jumpPressed = isActionPressed(inputManager, GameAction::Jump);
        if (jumpPressed && zTransforms.has(player)) {
            auto& zComp = zTransforms.get(player);
            if (zComp.isGrounded()) {
                zComp.jump(800.0f);
                std::cout << "[Jump] Player jumped! vz=800\n";
            }
        }
        
        // 丢炸弹输入
        bool bombPressed = isActionPressed(inputManager, GameAction::DropBomb);
        
        static float bombCooldown = 0.0f;
        if (bombCooldown > 0.0f) {
            bombCooldown -= dtTime.asSeconds();
        }
        
        if (bombPressed && !lastBombPressed && bombCooldown <= 0.0f) {
            Entity bomb = ecs.create();
            
            std::cout << "[Bomb] 📦 放置炸弹！ID=" << (uint32_t)bomb << " CD=0.5s\n";
            bombCooldown = 0.5f;
                
            const auto& playerTrans = transforms.get(player);
            const auto& playerZ = zTransforms.get(player);
            
            float facingX = playerTrans.facingX;
            float facingY = playerTrans.facingY;
            
            if (facingX == 0.0f && facingY == 0.0f) {
                const auto& input = inputs.get(player);
                if (input.moveDir.x != 0.0f || input.moveDir.y != 0.0f) {
                    facingX = input.moveDir.x;
                    facingY = input.moveDir.y;
                    float len = std::sqrt(facingX * facingX + facingY * facingY);
                    if (len > 0.0f) {
                        facingX /= len;
                        facingY /= len;
                    }
                } else {
                    facingX = 1.0f;
                    facingY = 0.0f;
                }
            }
            
            float offsetX = facingX * 35.0f;
            float offsetY = facingY * 35.0f;
            
            transforms.add(bomb, {
                .position = {playerTrans.position.x + offsetX, playerTrans.position.y + offsetY},
                .scale = {1.0f, 1.0f},
                .rotation = 0.0f,
                .velocity = {0.0f, 0.0f}
            });
            
            bombs.add(bomb, {
                .fuseTimer = 3.0f,
                .isKicked = false
            });
            
            zTransforms.add(bomb, {
                .z = 20.0f,
                .vz = 300.0f,
                .gravity = -1500.0f,
                .height = 30.0f
            });
            
            colliders.add(bomb, {
                .radius = 12.0f,
                .isStatic = false,
                .mass = 1.0f
            });
            
            std::cout << "[Bomb] 丢出炸弹！fuse=3.0s pos=(" 
                      << (playerTrans.position.x + offsetX) << ", "
                      << (playerTrans.position.y + offsetY) << ")\n";
        }
        lastBombPressed = bombPressed;

        // --- 神圣的 Fixed Timestep 物理循环 ---
        while (timeSinceLastUpdate >= TIME_PER_FRAME) {
            timeSinceLastUpdate -= TIME_PER_FRAME;
            float fixedDt = TIME_PER_FRAME.asSeconds() * timeScale;
            
            if (frameStep) {
                frameStep = false;
                timeScale = 0.0f;
                std::cout << "[FrameStep] Single frame executed\n";
            }
            
            stateSystem.update(states, attackStates, inputs, damageEvents, ecs, fixedDt);
            dashSystem.update(dashes, states, transforms, inputs, fixedDt);
            locomotionSystem.update(states, transforms, characters, inputs, fixedDt);
            magnetSystem.update(transforms, magnets, transforms, itemDatas, fixedDt);
            bombSystem.update(bombs, transforms, zTransforms, states, characters, hitboxes, lifetimes, transforms, deathTags, ecs, fixedDt);
            
            for (Entity entity : zTransforms.entityList()) {
                if (zTransforms.has(entity)) {
                    auto& zTrans = zTransforms.get(entity);
                    zTrans.applyGravity(fixedDt);
                    
                    if (zTrans.z <= 0.0f) {
                        zTrans.z = 0.0f;
                        
                        if (states.has(entity)) {
                            auto& state = states.get(entity);
                            if (state.currentState == CharacterState::KnockedAirborne) {
                                state.currentState = CharacterState::Idle;
                                state.previousState = CharacterState::Idle;
                                std::cout << "[ZPhysics] 🛬 落地恢复！Entity " << entity << " → Idle\n";
                            }
                        }
                        
                        if (zTrans.vz < 0.0f && std::abs(zTrans.vz) > 50.0f) {
                            zTrans.vz = -zTrans.vz * 0.5f;
                        } else {
                            zTrans.vz = 0.0f;
                        }
                    }
                }
            }
            
            movementSystem.update(transforms, itemDatas, states, bombs, zTransforms, fixedDt);
            physicalCollisionSystem.update(colliders, transforms, fixedDt);
            attachmentSystem.update(attachedComponents, transforms, zTransforms, fixedDt);
            attackSystem.update(states, attackStates, transforms, characters, ecs, hitboxes, lifetimes, attachedComponents, zTransforms, fixedDt);
            collisionSystem.update(hitboxes, hurtboxes, transforms, transforms, zTransforms, damageEvents, ecs, fixedDt);
            damageSystem.update(characters, damageEvents, deathTags, states, dashes, transforms, zTransforms, damageTexts, lifetimes, ecs);
            lootSpawnSystem.update(transforms, lootDrops, itemDatas, pickupBoxes, deathTags, ecs);
            deathSystem.update(states, transforms, characters, hurtboxes, lootDrops, inputs, deathTags, ecs, evolutions, fixedDt);
            pickupSystem.update(ecs, evolutions, transforms, transforms, itemDatas, pickupBoxes, magnets);
            
            if (states.get(player).currentState == CharacterState::Dead) {
                respawnTimer += fixedDt;
                if (respawnTimer >= 2.0f) {
                    characters.get(player).currentHP = characters.get(player).maxHP;
                    states.get(player).currentState = CharacterState::Idle;
                    states.get(player).previousState = CharacterState::Idle;
                    states.get(player).stateTimer = 0.0f;
                    transforms.get(player).position = {200.0f, 300.0f};
                    transforms.get(player).velocity = {0.0f, 0.0f};
                    respawnTimer = 0.0f;
                    std::cout << "[Respawn] Player revived! HP=100, pos=(200,300)\n";
                }
            }
        }
        
        cleanupSystem.update(ecs, deathTags, lifetimes,
            states, transforms, characters, inputs, hurtboxes,
            hitboxes, attackStates, damageTags,
            lootDrops, itemDatas, pickupBoxes,
            magnets, evolutions, dashes,
            bombs, attachedComponents, colliders,
            zTransforms,
            damageTexts,
            TIME_PER_FRAME.asSeconds());
        
        // --- 循环外：纯渲染 ---
        window.clear(COLOR_BACKGROUND);
        renderGrid(window);
        
        std::vector<Entity> renderOrder;
        for (Entity entity : characters.entityList()) {
            if (transforms.has(entity) && states.has(entity)) {
                renderOrder.push_back(entity);
            }
        }
        
        std::sort(renderOrder.begin(), renderOrder.end(), [&transforms](Entity a, Entity b) {
            return transforms.get(a).position.y < transforms.get(b).position.y;
        });
        
        for (Entity entity : renderOrder) {
            bool isPlayer = (entity == player);
            const DashComponent* dashPtr = dashes.has(entity) ? &dashes.get(entity) : nullptr;
            const ZTransformComponent* zPtr = zTransforms.has(entity) ? &zTransforms.get(entity) : nullptr;
            
            renderEntity(window, transforms.get(entity), characters.get(entity), states.get(entity), isPlayer, dashPtr, zPtr);
        }
        
        renderHitboxes(window, transforms, hitboxes, zTransforms);
        renderLoot(window, transforms, itemDatas);
        renderBombs(window, transforms, bombs, zTransforms);
        damageTextRenderSystem.update(damageTexts, window, font, ecs, TIME_PER_FRAME.asSeconds());
        
        // ========== UI 帮助面板渲染（动态菜单）==========
        if (showHelpUI) {
            // 1. 画半透明黑色背景板（加大尺寸容纳菜单）
            sf::RectangleShape overlay({420.0f, 400.0f});
            overlay.setPosition(sf::Vector2f(10.0f, 10.0f));
            overlay.setFillColor(sf::Color(0, 0, 0, 200));
            overlay.setOutlineColor(sf::Color(255, 255, 255, 255));
            overlay.setOutlineThickness(2.0f);
            window.draw(overlay);
            
            // 2. 画标题
            sf::Text titleText(font, "--- Settings: Key Bindings ---", 18);
            titleText.setFillColor(sf::Color::Cyan);
            titleText.setPosition({20.0f, 20.0f});
            window.draw(titleText);
            
            // 3. 循环画每一行按键配置（逐行渲染实现高亮）
            float startY = 60.0f;
            for (size_t i = 0; i < bindableActions.size(); ++i) {
                GameAction action = bindableActions[i];
                bool isSelected = (static_cast<int>(i) == selectedMenuIndex);
                
                std::string rowText = isSelected ? "> " : "  "; // 选中行前面加箭头
                rowText += actionNames[action] + " : ";
                
                // 如果正在修改当前行，显示闪烁提示
                if (isSelected && isRebinding) {
                    // 闪烁效果：根据时间切换显示
                    float blinkSpeed = 8.0f;
                    float alpha = std::sin(clock.getElapsedTime().asSeconds() * blinkSpeed);
                    if (alpha > 0) {
                        rowText += "[ PRESS ANY KEY... ]";
                    } else {
                        rowText += "[                 ]";
                    }
                } else {
                    rowText += "[" + inputManager.keyToString(inputManager.getMappedKey(action)) + "]";
                }
                
                sf::Text itemText(font, rowText, 16);
                itemText.setPosition({20.0f, startY + static_cast<float>(i) * 28.0f});
                
                // 选中行显示黄色，否则白色
                itemText.setFillColor(isSelected ? sf::Color::Yellow : sf::Color::White);
                window.draw(itemText);
            }
            
            // 4. 画底部操作提示
            sf::Text hintText(font, "Use W/S or ↑/↓ to select | ENTER to rebind | ESC to cancel | F1 to close", 13);
            hintText.setFillColor(sf::Color(150, 150, 150));
            hintText.setPosition({20.0f, startY + static_cast<float>(bindableActions.size()) * 28.0f + 25.0f});
            window.draw(hintText);
        }
        
        window.display();
        
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

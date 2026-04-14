/**
 * @file VisualSandbox.cpp
 * @brief 可视化调试沙盒 - 战斗管线测试（含冲刺与战利品系统）
 *
 * ✅ ECS 架构大清洗：GameWorld 注册表模式
 * ✅ 双玩家改键：InputManager 挂载到 GameWorld
 * ✅ ImGui 集成 + 双轨制混合输入（P1 字典映射 + P2 原生摇杆）
 */

#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <iostream>
#include <string>
#include <vector>
#include <cmath>
#include <algorithm>
#include <unordered_map>

// ========== ImGui ==========
#include "imgui.h"
#include "imgui-SFML.h"
#include <filesystem>

// ========== 核心输入管理器 ==========
#include "core/InputManager.h"

// ========== GameWorld ==========
#include "core/GameWorld.h"
#include "core/EntityFactory.h"

// ========== 所有系统 ==========
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
#include "systems/DamageTextSpawnerSystem.h"
#include "systems/DamageTextRenderSystem.h"
#include "systems/RenderSystem.h"

// ========== 平台适配器 ==========

sf::Keyboard::Key toSFMLKey(EngineKey eKey) {
    switch(eKey) {
        // 26 个字母键
        case EngineKey::A: return sf::Keyboard::Key::A;
        case EngineKey::B: return sf::Keyboard::Key::B;
        case EngineKey::C: return sf::Keyboard::Key::C;
        case EngineKey::D: return sf::Keyboard::Key::D;
        case EngineKey::E: return sf::Keyboard::Key::E;
        case EngineKey::F: return sf::Keyboard::Key::F;
        case EngineKey::G: return sf::Keyboard::Key::G;
        case EngineKey::H: return sf::Keyboard::Key::H;
        case EngineKey::I: return sf::Keyboard::Key::I;
        case EngineKey::J: return sf::Keyboard::Key::J;
        case EngineKey::K: return sf::Keyboard::Key::K;
        case EngineKey::L: return sf::Keyboard::Key::L;
        case EngineKey::M: return sf::Keyboard::Key::M;
        case EngineKey::N: return sf::Keyboard::Key::N;
        case EngineKey::O: return sf::Keyboard::Key::O;
        case EngineKey::P: return sf::Keyboard::Key::P;
        case EngineKey::Q: return sf::Keyboard::Key::Q;
        case EngineKey::R: return sf::Keyboard::Key::R;
        case EngineKey::S: return sf::Keyboard::Key::S;
        case EngineKey::T: return sf::Keyboard::Key::T;
        case EngineKey::U: return sf::Keyboard::Key::U;
        case EngineKey::V: return sf::Keyboard::Key::V;
        case EngineKey::W: return sf::Keyboard::Key::W;
        case EngineKey::X: return sf::Keyboard::Key::X;
        case EngineKey::Y: return sf::Keyboard::Key::Y;
        case EngineKey::Z: return sf::Keyboard::Key::Z;
        // 功能/修饰键
        case EngineKey::Space: return sf::Keyboard::Key::Space;
        case EngineKey::Escape: return sf::Keyboard::Key::Escape;
        case EngineKey::Backspace: return sf::Keyboard::Key::Backspace;
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
        // 标点/符号键
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
        // 编辑键
        case EngineKey::Delete: return sf::Keyboard::Key::Delete;
        case EngineKey::Insert: return sf::Keyboard::Key::Insert;
        case EngineKey::Home: return sf::Keyboard::Key::Home;
        case EngineKey::End: return sf::Keyboard::Key::End;
        case EngineKey::PageUp: return sf::Keyboard::Key::PageUp;
        case EngineKey::PageDown: return sf::Keyboard::Key::PageDown;
        // 方向键
        case EngineKey::Up: return sf::Keyboard::Key::Up;
        case EngineKey::Down: return sf::Keyboard::Key::Down;
        case EngineKey::Left: return sf::Keyboard::Key::Left;
        case EngineKey::Right: return sf::Keyboard::Key::Right;
        // 小键盘
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
        // 鼠标键（无对应键盘键）
        case EngineKey::MouseLeft: return sf::Keyboard::Key::Unknown;
        case EngineKey::MouseRight: return sf::Keyboard::Key::Unknown;
        case EngineKey::MouseMiddle: return sf::Keyboard::Key::Unknown;
        case EngineKey::MouseX1: return sf::Keyboard::Key::Unknown;
        case EngineKey::MouseX2: return sf::Keyboard::Key::Unknown;
        default: return sf::Keyboard::Key::Unknown;
    }
}

EngineKey toEngineKey(sf::Keyboard::Key sfKey) {
    switch(sfKey) {
        // 26 个字母键
        case sf::Keyboard::Key::A: return EngineKey::A;
        case sf::Keyboard::Key::B: return EngineKey::B;
        case sf::Keyboard::Key::C: return EngineKey::C;
        case sf::Keyboard::Key::D: return EngineKey::D;
        case sf::Keyboard::Key::E: return EngineKey::E;
        case sf::Keyboard::Key::F: return EngineKey::F;
        case sf::Keyboard::Key::G: return EngineKey::G;
        case sf::Keyboard::Key::H: return EngineKey::H;
        case sf::Keyboard::Key::I: return EngineKey::I;
        case sf::Keyboard::Key::J: return EngineKey::J;
        case sf::Keyboard::Key::K: return EngineKey::K;
        case sf::Keyboard::Key::L: return EngineKey::L;
        case sf::Keyboard::Key::M: return EngineKey::M;
        case sf::Keyboard::Key::N: return EngineKey::N;
        case sf::Keyboard::Key::O: return EngineKey::O;
        case sf::Keyboard::Key::P: return EngineKey::P;
        case sf::Keyboard::Key::Q: return EngineKey::Q;
        case sf::Keyboard::Key::R: return EngineKey::R;
        case sf::Keyboard::Key::S: return EngineKey::S;
        case sf::Keyboard::Key::T: return EngineKey::T;
        case sf::Keyboard::Key::U: return EngineKey::U;
        case sf::Keyboard::Key::V: return EngineKey::V;
        case sf::Keyboard::Key::W: return EngineKey::W;
        case sf::Keyboard::Key::X: return EngineKey::X;
        case sf::Keyboard::Key::Y: return EngineKey::Y;
        case sf::Keyboard::Key::Z: return EngineKey::Z;
        // 数字键
        case sf::Keyboard::Key::Num0: return EngineKey::Num0;
        case sf::Keyboard::Key::Num1: return EngineKey::Num1;
        case sf::Keyboard::Key::Num2: return EngineKey::Num2;
        case sf::Keyboard::Key::Num3: return EngineKey::Num3;
        case sf::Keyboard::Key::Num4: return EngineKey::Num4;
        case sf::Keyboard::Key::Num5: return EngineKey::Num5;
        case sf::Keyboard::Key::Num6: return EngineKey::Num6;
        case sf::Keyboard::Key::Num7: return EngineKey::Num7;
        case sf::Keyboard::Key::Num8: return EngineKey::Num8;
        case sf::Keyboard::Key::Num9: return EngineKey::Num9;
        // 功能键
        case sf::Keyboard::Key::F1:  return EngineKey::F1;
        case sf::Keyboard::Key::F2:  return EngineKey::F2;
        case sf::Keyboard::Key::F3:  return EngineKey::F3;
        case sf::Keyboard::Key::F4:  return EngineKey::F4;
        case sf::Keyboard::Key::F5:  return EngineKey::F5;
        case sf::Keyboard::Key::F6:  return EngineKey::F6;
        case sf::Keyboard::Key::F7:  return EngineKey::F7;
        case sf::Keyboard::Key::F8:  return EngineKey::F8;
        case sf::Keyboard::Key::F9:  return EngineKey::F9;
        case sf::Keyboard::Key::F10: return EngineKey::F10;
        case sf::Keyboard::Key::F11: return EngineKey::F11;
        case sf::Keyboard::Key::F12: return EngineKey::F12;
        // 修饰键
        case sf::Keyboard::Key::Space:     return EngineKey::Space;
        case sf::Keyboard::Key::Escape:    return EngineKey::Escape;
        case sf::Keyboard::Key::Backspace: return EngineKey::Backspace;
        case sf::Keyboard::Key::Enter:     return EngineKey::Enter;
        case sf::Keyboard::Key::LShift:    return EngineKey::LeftShift;
        case sf::Keyboard::Key::RShift:    return EngineKey::RightShift;
        case sf::Keyboard::Key::LControl:  return EngineKey::LeftCtrl;
        case sf::Keyboard::Key::RControl:  return EngineKey::RightCtrl;
        case sf::Keyboard::Key::LAlt:      return EngineKey::LeftAlt;
        case sf::Keyboard::Key::RAlt:      return EngineKey::RightAlt;
        // 方向键
        case sf::Keyboard::Key::Up:    return EngineKey::Up;
        case sf::Keyboard::Key::Down:  return EngineKey::Down;
        case sf::Keyboard::Key::Left:  return EngineKey::Left;
        case sf::Keyboard::Key::Right: return EngineKey::Right;
        // 标点/符号键
        case sf::Keyboard::Key::Comma:      return EngineKey::Comma;
        case sf::Keyboard::Key::Period:     return EngineKey::Period;
        case sf::Keyboard::Key::Slash:      return EngineKey::Slash;
        case sf::Keyboard::Key::Semicolon:  return EngineKey::SemiColon;
        case sf::Keyboard::Key::Apostrophe: return EngineKey::Quote;
        case sf::Keyboard::Key::LBracket:   return EngineKey::LeftBracket;
        case sf::Keyboard::Key::RBracket:   return EngineKey::RightBracket;
        case sf::Keyboard::Key::Backslash:  return EngineKey::Backslash;
        case sf::Keyboard::Key::Hyphen:     return EngineKey::Minus;
        case sf::Keyboard::Key::Equal:      return EngineKey::Equal;
        case sf::Keyboard::Key::Grave:      return EngineKey::Backquote;
        // 编辑键
        case sf::Keyboard::Key::Delete:  return EngineKey::Delete;
        case sf::Keyboard::Key::Insert:  return EngineKey::Insert;
        case sf::Keyboard::Key::Home:    return EngineKey::Home;
        case sf::Keyboard::Key::End:     return EngineKey::End;
        case sf::Keyboard::Key::PageUp:  return EngineKey::PageUp;
        case sf::Keyboard::Key::PageDown:return EngineKey::PageDown;
        // 小键盘
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

EngineKey toEngineKey(sf::Mouse::Button mb) {
    switch(mb) {
        case sf::Mouse::Button::Left: return EngineKey::MouseLeft;
        case sf::Mouse::Button::Right: return EngineKey::MouseRight;
        case sf::Mouse::Button::Middle: return EngineKey::MouseMiddle;
        default: return EngineKey::Unknown;
    }
}

EngineKey toEngineKey(unsigned int joystickBtn) {
    switch(joystickBtn) {
        case 0:  return EngineKey::JoyBtn0;   case 1:  return EngineKey::JoyBtn1;
        case 2:  return EngineKey::JoyBtn2;   case 3:  return EngineKey::JoyBtn3;
        case 4:  return EngineKey::JoyBtn4;   case 5:  return EngineKey::JoyBtn5;
        case 6:  return EngineKey::JoyBtn6;   case 7:  return EngineKey::JoyBtn7;
        case 8:  return EngineKey::JoyBtn8;   case 9:  return EngineKey::JoyBtn9;
        case 10: return EngineKey::JoyBtn10;  case 11: return EngineKey::JoyBtn11;
        case 12: return EngineKey::JoyBtn12;  case 13: return EngineKey::JoyBtn13;
        case 14: return EngineKey::JoyBtn14;  case 15: return EngineKey::JoyBtn15;
        default: return EngineKey::Unknown;
    }
}

/**
 * @brief 检查指定玩家的某个动作是否被按下（键盘 + 鼠标 + 手柄按钮）
 */
bool isActionPressed(const InputManager& input, PlayerIndex player, GameAction action) {
    EngineKey eKey = input.getMappedKey(player, action);
    if (eKey == EngineKey::MouseLeft) return sf::Mouse::isButtonPressed(sf::Mouse::Button::Left);
    if (eKey == EngineKey::MouseRight) return sf::Mouse::isButtonPressed(sf::Mouse::Button::Right);
    if (eKey == EngineKey::MouseMiddle) return sf::Mouse::isButtonPressed(sf::Mouse::Button::Middle);

    // 手柄按钮检测：遍历所有已连接的手柄
    if (static_cast<int>(eKey) >= static_cast<int>(EngineKey::JoyBtn0) &&
        static_cast<int>(eKey) <= static_cast<int>(EngineKey::JoyBtn15)) {
        int buttonIndex = static_cast<int>(eKey) - static_cast<int>(EngineKey::JoyBtn0);
        for (unsigned int i = 0; i < sf::Joystick::Count; ++i) {
            if (sf::Joystick::isConnected(i) && sf::Joystick::isButtonPressed(i, buttonIndex)) {
                return true;
            }
        }
        return false;
    }

    return sf::Keyboard::isKeyPressed(toSFMLKey(eKey));
}

// ========== 渲染配置 ==========
constexpr int WINDOW_WIDTH = 1600;
constexpr int WINDOW_HEIGHT = 900;

// ========== 地图 & 围栏 ==========
constexpr float MAP_WIDTH  = 2000.0f;
constexpr float MAP_HEIGHT = 1200.0f;
constexpr float FENCE_RADIUS = 20.0f;
constexpr float FENCE_SPACING = FENCE_RADIUS * 1.6f;

constexpr float ENTITY_SIZE = 40.0f;

sf::Color COLOR_PLAYER(50, 200, 50);
sf::Color COLOR_ENEMY(200, 50, 50);
sf::Color COLOR_HURT(255, 100, 100);
sf::Color COLOR_DEAD(100, 100, 100);
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

// ========== 围栏工厂 ==========
void createFence(GameWorld& world) {
    float fenceRadius = FENCE_RADIUS;
    float spacing = FENCE_SPACING;

    // 水平围栏：走满地图宽度
    auto addRow = [&](float y, float xStart, float xEnd) {
        for (float x = xStart; x <= xEnd + 0.5f; x += spacing) {
            Entity ball = world.ecs.create();
            world.transforms.add(ball, {{x, y}, {1.0f, 1.0f}, 0.0f, {0.0f, 0.0f}, 1.0f, 0.0f});
            world.colliders.add(ball, {fenceRadius, true});
            world.fenceBalls.push_back(ball);
        }
    };

    // 垂直围栏：两端缩进 fenceRadius 避免和水平围栏角重叠
    auto addCol = [&](float x, float yStart, float yEnd) {
        for (float y = yStart; y <= yEnd + 0.5f; y += spacing) {
            Entity ball = world.ecs.create();
            world.transforms.add(ball, {{x, y}, {1.0f, 1.0f}, 0.0f, {0.0f, 0.0f}, 1.0f, 0.0f});
            world.colliders.add(ball, {fenceRadius, true});
            world.fenceBalls.push_back(ball);
        }
    };

    // 顶部 & 底部
    addRow(0.0f, 0.0f, MAP_WIDTH);
    addRow(MAP_HEIGHT, 0.0f, MAP_WIDTH);
    // 左侧 & 右侧（缩进）
    addCol(0.0f, fenceRadius, MAP_HEIGHT - fenceRadius);
    addCol(MAP_WIDTH, fenceRadius, MAP_HEIGHT - fenceRadius);
}

// ========== 改键状态机（ImGui 用） ==========
struct RebindState {
    PlayerIndex player{PlayerIndex::P1};
    GameAction action{GameAction::Up};
    bool active{false};
};

// ========== 主函数 ==========

int main() {
    std::cout << "=== Project2 GameWorld Sandbox (ImGui Hybrid Input) ===\n";

    sf::RenderWindow window(sf::VideoMode({WINDOW_WIDTH, WINDOW_HEIGHT}), "Project2 GameWorld Sandbox");
    window.setFramerateLimit(144);

    // ImGui 初始化
    bool imguiInitOk = ImGui::SFML::Init(window);
    std::cout << "[ImGui] Init returned: " << (imguiInitOk ? "true (OK)" : "false (FAILED!)") << std::endl;
    if (!imguiInitOk) {
        std::cerr << "[ImGui] *** INIT FAILED — ImGui will NOT work! ***" << std::endl;
    }

    // 加载中文字体（解决乱码）
    // 基于 __FILE__ 编译期路径，不受 CWD 影响
    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->Clear();
    std::filesystem::path srcDir = std::filesystem::path(__FILE__).parent_path();
    std::filesystem::path projectRoot = srcDir.parent_path().parent_path(); // sandbox → tests → Project2
    std::filesystem::path fontPath = projectRoot / "material" / "fonts" / "NotoSansSC-Regular.otf";
    std::string fontStr = fontPath.string();
    ImFont* cjkFont = io.Fonts->AddFontFromFileTTF(
        fontStr.c_str(), 16.0f, nullptr,
        io.Fonts->GetGlyphRangesChineseFull());
    if (!cjkFont) {
        io.Fonts->AddFontDefault();
    }
    ImGui::SFML::UpdateFontTexture();

    float timeScale = 1.0f;
    bool frameStep = false;

    // ✅ 单一世界上下文（InputManager 构造函数已自动加载配置）
    GameWorld world;

    // 实例化所有系统
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
    RenderSystem renderSystem;

    sf::Font font;
    if (!font.openFromFile("/System/Library/Fonts/Supplemental/Arial Unicode.ttf")) {
        font.openFromFile("/System/Library/Fonts/Helvetica.ttc");
    }

    // ========== 数据驱动工厂 ==========
    EntityFactory factory;
    factory.loadPrefabs("data/prefabs.json");

    world.player1 = factory.spawnPlayer(world, 500, 600);
    world.player2 = factory.spawnPlayer(world, 1500, 600);

    // 创建围栏
    createFence(world);

    const sf::Time TIME_PER_FRAME = sf::seconds(1.0f / 60.0f);
    sf::Time timeSinceLastUpdate = sf::Time::Zero;
    sf::Clock clock, debugClock;

    // ImGui 面板状态
    bool showImGuiPanel = false;
    bool lastF1Pressed = false;
    bool f1Pressed = false;
    RebindState rebindState;

    // 动作按钮列表
    std::vector<GameAction> actionButtons = {
        GameAction::Attack, GameAction::Dash, GameAction::DropBomb,
        GameAction::Jump, GameAction::Pause, GameAction::FrameStep
    };
    std::vector<GameAction> moveButtons = {
        GameAction::Up, GameAction::Down, GameAction::Left, GameAction::Right
    };

    // 用于检测 P2 手柄连接的变量
    bool lastP2JoystickConnected = sf::Joystick::isConnected(0);

    // ========== 动态相机状态（大乱斗风格） ==========
    // 初始位置为玩家出生点中点: P1(500,600) + P2(1500,600) → (1000,600)
    static sf::Vector2f currentCameraCenter{1000.0f, 600.0f};
    static float currentCameraSize = 1200.0f;

    // 重置时钟，避免从 clock 初始化到进入主循环之间的 accumulated time spike
    clock.restart();

    while (window.isOpen()) {
        // ========== 动态相机计算（在 pollEvent 之前设置 view） ==========
        {
            float cameraDt = clock.getElapsedTime().asSeconds();
            if (cameraDt > 0.1f) cameraDt = 0.1f; // cap initial spike
            if (cameraDt <= 0.0f) cameraDt = 1.0f / 144.0f;

            std::vector<sf::Vector2f> alivePositions;

            auto collectAlive = [&](Entity player) {
                if (world.states.has(player) && world.transforms.has(player)) {
                    auto& state = world.states.get(player);
                    if (state.currentState != CharacterState::Dead) {
                        auto& tr = world.transforms.get(player);
                        alivePositions.emplace_back(tr.position.x, tr.position.y);
                    }
                }
            };
            collectAlive(world.player1);
            collectAlive(world.player2);

            sf::Vector2f targetCenter{1000.0f, 600.0f};
            float targetSize = 1200.0f;

            if (alivePositions.size() == 1) {
                // 单人存活：锁定玩家，固定视野
                targetCenter = alivePositions[0];
                targetSize = 800.0f;
            } else if (alivePositions.size() >= 2) {
                // 双人存活：计算包围盒
                float minX = alivePositions[0].x, maxX = alivePositions[0].x;
                float minY = alivePositions[0].y, maxY = alivePositions[0].y;
                for (const auto& p : alivePositions) {
                    if (p.x < minX) minX = p.x;
                    if (p.x > maxX) maxX = p.x;
                    if (p.y < minY) minY = p.y;
                    if (p.y > maxY) maxY = p.y;
                }
                float width = maxX - minX;
                float height = maxY - minY;
                targetCenter = sf::Vector2f((minX + maxX) / 2.0f, (minY + maxY) / 2.0f);
                float aspectHeight = height * 16.0f / 9.0f;
                targetSize = std::max(width, aspectHeight) + 300.0f;
            }

            // 限制 targetSize 范围
            targetSize = std::clamp(targetSize, 600.0f, 2000.0f);

            // lerp 平滑过渡
            float lerpSpeed = 12.0f * cameraDt;
            float t = std::clamp(lerpSpeed, 0.0f, 1.0f);
            currentCameraCenter.x += (targetCenter.x - currentCameraCenter.x) * t;
            currentCameraCenter.y += (targetCenter.y - currentCameraCenter.y) * t;
            currentCameraSize += (targetSize - currentCameraSize) * t;

            // 应用 view
            float viewHeight = static_cast<float>(WINDOW_HEIGHT) / static_cast<float>(WINDOW_WIDTH) * currentCameraSize;
            sf::View dynamicView;
            dynamicView.setCenter(currentCameraCenter);
            dynamicView.setSize({currentCameraSize, viewHeight});
            window.setView(dynamicView);
        }

        // ========== 事件处理 ==========
        while (const auto event = window.pollEvent()) {
            // ========== ImGui 事件处理（最高优先级） ==========
            ImGui::SFML::ProcessEvent(window, *event);

            if (event->is<sf::Event::Closed>()) window.close();

            // ========== F1 原生事件检测（绕过 imgui-SFML 兼容性问题） ==========
            if (const auto* kp = event->getIf<sf::Event::KeyPressed>()) {
                if (kp->code == sf::Keyboard::Key::F1) {
                    f1Pressed = true;
                }
            }

            // ========== 改键拦截（ImGui 面板中触发） ==========
            if (rebindState.active && event->is<sf::Event::KeyPressed>()) {
                const auto* kp = event->getIf<sf::Event::KeyPressed>();
                std::cout << "[Rebind-Debug] KeyPressed: sfKey=" << static_cast<int>(kp->code) << std::endl;
                EngineKey pressedKey = toEngineKey(kp->code);
                std::cout << "[Rebind-Debug]   mappedKey=" << world.inputManager.keyToString(pressedKey) << std::endl;

                // Escape: 取消当前改键操作
                if (pressedKey == EngineKey::Escape) {
                    std::cout << "[Rebind] Cancelled (Escape)" << std::endl;
                    rebindState.active = false;
                    continue;
                }

                // Delete / Backspace: 清除该动作的绑定（设为 Unknown）
                if (pressedKey == EngineKey::Delete || pressedKey == EngineKey::Backspace) {
                    world.inputManager.setMappedKey(rebindState.player, rebindState.action, EngineKey::Unknown);
                    std::cout << "[Rebind] [" << (rebindState.player == PlayerIndex::P1 ? "P1" : "P2") << "] "
                              << world.inputManager.actionToString(rebindState.action)
                              << " -> [Cleared]" << std::endl;
                    rebindState.active = false;
                    continue;
                }

                if (pressedKey == EngineKey::Unknown) {
                    std::cout << "[Rebind-Warn] Key not mapped to any EngineKey (sfKey=" << static_cast<int>(kp->code) << ")" << std::endl;
                    rebindState.active = false;
                    continue;
                }

                // 冲突检测：直接解除旧绑定，新键绑定到新动作
                auto conflictOpt = world.inputManager.findActionByKey(rebindState.player, pressedKey);
                if (conflictOpt.has_value() && conflictOpt.value() != rebindState.action) {
                    GameAction oldAction = conflictOpt.value();
                    world.inputManager.setMappedKey(rebindState.player, oldAction, EngineKey::Unknown);
                    std::cout << "[Rebind-Override] [" << (rebindState.player == PlayerIndex::P1 ? "P1" : "P2") << "] "
                              << "解除 [" << world.inputManager.actionToString(oldAction) << "] 的绑定" << std::endl;
                }
                world.inputManager.setMappedKey(rebindState.player, rebindState.action, pressedKey);
                std::cout << "[Rebind] [" << (rebindState.player == PlayerIndex::P1 ? "P1" : "P2") << "] "
                          << world.inputManager.actionToString(rebindState.action)
                          << " -> " << world.inputManager.keyToString(pressedKey) << std::endl;
                rebindState.active = false;
                continue;
            }
            else if (rebindState.active && event->is<sf::Event::MouseButtonPressed>()) {
                const auto* mb = event->getIf<sf::Event::MouseButtonPressed>();
                EngineKey pressedKey = toEngineKey(mb->button);
                if (pressedKey == EngineKey::Unknown) {
                    std::cout << "[Rebind-Warn] Unknown mouse button" << std::endl;
                    rebindState.active = false;
                    continue;
                }

                // 冲突检测：直接解除旧绑定
                auto conflictOpt = world.inputManager.findActionByKey(rebindState.player, pressedKey);
                if (conflictOpt.has_value() && conflictOpt.value() != rebindState.action) {
                    GameAction oldAction = conflictOpt.value();
                    world.inputManager.setMappedKey(rebindState.player, oldAction, EngineKey::Unknown);
                    std::cout << "[Rebind-Override] [" << (rebindState.player == PlayerIndex::P1 ? "P1" : "P2") << "] "
                              << "解除 [" << world.inputManager.actionToString(oldAction) << "] 的绑定" << std::endl;
                }
                world.inputManager.setMappedKey(rebindState.player, rebindState.action, pressedKey);
                std::cout << "[Rebind] [" << (rebindState.player == PlayerIndex::P1 ? "P1" : "P2") << "] "
                          << world.inputManager.actionToString(rebindState.action)
                          << " -> " << world.inputManager.keyToString(pressedKey) << std::endl;
                rebindState.active = false;
                continue;
            }
            else if (rebindState.active && event->is<sf::Event::JoystickButtonPressed>()) {
                const auto* jb = event->getIf<sf::Event::JoystickButtonPressed>();
                EngineKey pressedKey = toEngineKey(jb->button);
                std::cout << "[Rebind-Debug] JoystickButtonPressed: joystickId=" << jb->joystickId
                          << " button=" << jb->button << " mappedKey=" << world.inputManager.keyToString(pressedKey) << std::endl;
                if (pressedKey == EngineKey::Unknown) {
                    std::cout << "[Rebind-Warn] Unknown joystick button" << std::endl;
                    rebindState.active = false;
                    continue;
                }

                // 冲突检测：直接解除旧绑定
                auto conflictOpt = world.inputManager.findActionByKey(rebindState.player, pressedKey);
                if (conflictOpt.has_value() && conflictOpt.value() != rebindState.action) {
                    GameAction oldAction = conflictOpt.value();
                    world.inputManager.setMappedKey(rebindState.player, oldAction, EngineKey::Unknown);
                    std::cout << "[Rebind-Override] [" << (rebindState.player == PlayerIndex::P1 ? "P1" : "P2") << "] "
                              << "解除 [" << world.inputManager.actionToString(oldAction) << "] 的绑定" << std::endl;
                }
                world.inputManager.setMappedKey(rebindState.player, rebindState.action, pressedKey);
                std::cout << "[Rebind] [" << (rebindState.player == PlayerIndex::P1 ? "P1" : "P2") << "] "
                          << world.inputManager.actionToString(rebindState.action)
                          << " -> " << world.inputManager.keyToString(pressedKey) << std::endl;
                rebindState.active = false;
                continue;
            }

            if (const auto* kp = event->getIf<sf::Event::KeyPressed>()) {
                if (kp->code == sf::Keyboard::Key::Escape) window.close();
            }

            // 生成假人
            if (const auto* mb = event->getIf<sf::Event::MouseButtonPressed>()) {
                if (world.inputManager.getMappedKey(PlayerIndex::P1, GameAction::SpawnDummy) == EngineKey::MouseRight
                    && mb->button == sf::Mouse::Button::Right) {
                    sf::Vector2f worldPos = window.mapPixelToCoords(mb->position);
                    factory.spawnDummy(world, worldPos.x, worldPos.y);
                    std::cout << "[SpawnDummy] Dummy created at (" << worldPos.x << ", " << worldPos.y << ")\n";
                }
            }
        }

        // ========== ImGui 更新 ==========
        sf::Time deltaTime = clock.restart();
        ImGui::SFML::Update(window, deltaTime);

        // ========== F1 切换面板 ==========
        if (f1Pressed && !lastF1Pressed) {
            showImGuiPanel = !showImGuiPanel;
        }
        lastF1Pressed = f1Pressed;
        f1Pressed = false;  // 重置，等待下一帧事件

        // ========== ImGui 控制面板 ==========
        if (showImGuiPanel) {
            std::cout << "[ImGui-Render] About to call ImGui::Begin (panel is open)" << std::endl;
            if (ImGui::Begin("控制与调试 (Controls & Debug)")) {
                std::cout << "[ImGui-Render] ImGui::Begin returned true — drawing content" << std::endl;

            // ========== P1 设置面板（纯字典映射） ==========
            if (ImGui::CollapsingHeader("Player 1 - 键盘", ImGuiTreeNodeFlags_DefaultOpen)) {
                ImGui::Text("移动 (方向键)");
                for (GameAction action : moveButtons) {
                    ImGui::SameLine();
                    EngineKey key = world.inputManager.getMappedKey(PlayerIndex::P1, action);
                    std::string label = world.inputManager.actionToString(action) + "##move_p1_" + std::to_string(static_cast<int>(action));
                    std::string btnLabel;
                    if (rebindState.active && rebindState.player == PlayerIndex::P1 && rebindState.action == action) {
                        float blink = std::sin(ImGui::GetTime() * 8.0f);
                        btnLabel = blink > 0 ? "[ PRESS ANY KEY ]" : "[              ]";
                    } else {
                        btnLabel = world.inputManager.keyToString(key);
                    }
                    if (ImGui::Button(btnLabel.c_str())) {
                        rebindState = {PlayerIndex::P1, action, true};
                    }
                }
                ImGui::Separator();

                ImGui::Text("动作键");
                for (GameAction action : actionButtons) {
                    EngineKey key = world.inputManager.getMappedKey(PlayerIndex::P1, action);
                    std::string btnLabel;
                    if (rebindState.active && rebindState.player == PlayerIndex::P1 && rebindState.action == action) {
                        float blink = std::sin(ImGui::GetTime() * 8.0f);
                        btnLabel = blink > 0 ? "[ PRESS ANY KEY ]" : "[              ]";
                    } else {
                        btnLabel = world.inputManager.keyToString(key);
                    }
                    if (ImGui::Button(btnLabel.c_str())) {
                        rebindState = {PlayerIndex::P1, action, true};
                    }
                    ImGui::SameLine();
                    ImGui::TextDisabled(world.inputManager.actionToString(action).c_str());
                }
            }

            // ========== P2 设置面板（原生摇杆 + 字典动作） ==========
            if (ImGui::CollapsingHeader("Player 2 - 手柄", ImGuiTreeNodeFlags_DefaultOpen)) {
                bool p2Connected = sf::Joystick::isConnected(0);

                // 移动：原生模拟量，禁止改键
                ImGui::TextDisabled("左摇杆 : [ 硬件原生直连 (Native Analog) ]");
                if (p2Connected) {
                    float jx = sf::Joystick::getAxisPosition(0, sf::Joystick::Axis::X);
                    float jy = sf::Joystick::getAxisPosition(0, sf::Joystick::Axis::Y);
                    ImGui::SameLine();
                    ImGui::Text("X=%.1f Y=%.1f", jx, jy);
                } else {
                    ImGui::SameLine();
                    ImGui::TextDisabled("(未连接)");
                }

                ImGui::Separator();

                // 动作键：可改键
                ImGui::Text("动作映射");
                for (GameAction action : actionButtons) {
                    EngineKey key = world.inputManager.getMappedKey(PlayerIndex::P2, action);
                    std::string btnLabel;
                    if (rebindState.active && rebindState.player == PlayerIndex::P2 && rebindState.action == action) {
                        float blink = std::sin(ImGui::GetTime() * 8.0f);
                        btnLabel = blink > 0 ? "[ PRESS ANY KEY ]" : "[              ]";
                    } else {
                        btnLabel = world.inputManager.keyToString(key);
                    }
                    if (ImGui::Button(btnLabel.c_str())) {
                        rebindState = {PlayerIndex::P2, action, true};
                    }
                    ImGui::SameLine();
                    ImGui::TextDisabled(world.inputManager.actionToString(action).c_str());
                }
            }

            ImGui::Separator();

            // 保存按钮
            if (ImGui::Button("保存配置到 JSON", ImVec2(200, 30))) {
                world.inputManager.saveConfig();
            }
            ImGui::SameLine();
            if (ImGui::Button("重置为默认", ImVec2(200, 30))) {
                world.inputManager.resetToDefaults();
                std::cout << "[Rebind] All bindings reset to defaults" << std::endl;
            }

            ImGui::Separator();

            // 调试信息
            if (ImGui::CollapsingHeader("调试信息")) {
                float z1 = world.zTransforms.has(world.player1) ? world.zTransforms.get(world.player1).z : 0.0f;
                float z2 = world.zTransforms.has(world.player2) ? world.zTransforms.get(world.player2).z : 0.0f;
                ImGui::Text("P1: HP=%d State=%s Z=%.1f",
                    world.characters.get(world.player1).currentHP,
                    stateToString(world.states.get(world.player1).currentState).c_str(), z1);
                ImGui::Text("P2: HP=%d State=%s Z=%.1f",
                    world.characters.get(world.player2).currentHP,
                    stateToString(world.states.get(world.player2).currentState).c_str(), z2);
                ImGui::Text("实体数: %zu | 掉落物: %zu",
                    world.ecs.entities().size(), world.itemDatas.entityList().size());
                ImGui::Text("Juice: timeScale=%.2f shake=%.1f hitStop=%.3f",
                    world.juice.timeScale, world.juice.shakeIntensity, world.juice.hitStopTimer);
            }
        }
        ImGui::End();
        }

        // ========== 时间法则 ==========
        float realDt = deltaTime.asSeconds();

        if (world.juice.hitStopTimer > 0.0f) {
            world.juice.hitStopTimer -= realDt;
            if (world.juice.hitStopTimer <= 0.0f) {
                world.juice.timeScale = 1.0f;
                world.juice.hitStopTimer = 0.0f;
            } else {
                world.juice.timeScale = 0.0f;
            }
        }

        float gameDt = realDt * world.juice.timeScale;
        timeSinceLastUpdate += sf::seconds(gameDt);

        // ================================================================
        // ✅ 双轨制混合输入流
        // ================================================================

        // ========== [P1 轨道] 纯字典映射 ==========
        {
            Vec2 p1Dir{0.0f, 0.0f};
            bool up    = isActionPressed(world.inputManager, PlayerIndex::P1, GameAction::Up);
            bool down  = isActionPressed(world.inputManager, PlayerIndex::P1, GameAction::Down);
            bool left  = isActionPressed(world.inputManager, PlayerIndex::P1, GameAction::Left);
            bool right = isActionPressed(world.inputManager, PlayerIndex::P1, GameAction::Right);
            if (up)    p1Dir.y -= 1.0f;
            if (down)  p1Dir.y += 1.0f;
            if (left)  p1Dir.x -= 1.0f;
            if (right) p1Dir.x += 1.0f;
            if (float len = std::sqrt(p1Dir.x * p1Dir.x + p1Dir.y * p1Dir.y); len > 0.0f) {
                p1Dir.x /= len; p1Dir.y /= len;
            }

            auto& p1In = world.inputs.get(world.player1);
            p1In.moveDir = p1Dir;

            static bool lastP1Atk = false, lastP1Dash = false, lastP1Bomb = false;
            static float p1BombCD = 0.0f;

            bool p1Atk = isActionPressed(world.inputManager, PlayerIndex::P1, GameAction::Attack);
            if (p1Atk && !lastP1Atk) { p1In.pendingIntent = ActionIntent::Attack; p1In.intentTimer = 0.2f; }
            lastP1Atk = p1Atk;

            bool p1Dash = isActionPressed(world.inputManager, PlayerIndex::P1, GameAction::Dash);
            if (p1Dash && !lastP1Dash) { p1In.pendingIntent = ActionIntent::Dash; p1In.intentTimer = 0.2f; }
            lastP1Dash = p1Dash;

            if (isActionPressed(world.inputManager, PlayerIndex::P1, GameAction::Jump) && world.zTransforms.has(world.player1)) {
                auto& zc = world.zTransforms.get(world.player1);
                if (zc.isGrounded()) { zc.vz = 800.0f; }
            }

            p1BombCD = std::max(0.0f, p1BombCD - realDt);
            bool p1Bomb = isActionPressed(world.inputManager, PlayerIndex::P1, GameAction::DropBomb);
            if (p1Bomb && !lastP1Bomb && p1BombCD <= 0.0f) {
factory.spawnThrowableBomb(world, world.player1, world.inputs.get(world.player1));
                p1BombCD = 0.5f;
            }
            lastP1Bomb = p1Bomb;

            // P1 暂停/帧步进
            static bool lastP1Pause = false, lastP1Step = false;
            bool p1Pause = isActionPressed(world.inputManager, PlayerIndex::P1, GameAction::Pause);
            if (p1Pause && !lastP1Pause) {
                timeScale = (timeScale == 0.0f) ? 1.0f : 0.0f;
                std::cout << "[TimeScale] " << (timeScale == 0.0f ? "Paused" : "Running") << "\n";
            }
            lastP1Pause = p1Pause;

            bool p1Step = isActionPressed(world.inputManager, PlayerIndex::P1, GameAction::FrameStep);
            if (p1Step && !lastP1Step) { frameStep = true; timeScale = 1.0f; }
            lastP1Step = p1Step;
        }

        // ========== [P2 轨道] 原生摇杆模拟量 + 字典动作 ==========
        {
            Vec2 p2Dir{0.0f, 0.0f};

            if (sf::Joystick::isConnected(0)) {
                // 原生模拟量：直接读取摇杆轴
                float jx = sf::Joystick::getAxisPosition(0, sf::Joystick::Axis::X);
                float jy = sf::Joystick::getAxisPosition(0, sf::Joystick::Axis::Y);

                // 死区过滤
                if (std::abs(jx) > 20.0f) p2Dir.x = jx / 100.0f;
                if (std::abs(jy) > 20.0f) p2Dir.y = jy / 100.0f;

                // 限制最大长度（不归一化，保留轻推慢走）
                float len = std::sqrt(p2Dir.x * p2Dir.x + p2Dir.y * p2Dir.y);
                if (len > 1.0f) {
                    p2Dir.x /= len;
                    p2Dir.y /= len;
                }
            } else {
                // 键盘降级（通过 InputManager 查询）
                bool up    = isActionPressed(world.inputManager, PlayerIndex::P2, GameAction::Up);
                bool down  = isActionPressed(world.inputManager, PlayerIndex::P2, GameAction::Down);
                bool left  = isActionPressed(world.inputManager, PlayerIndex::P2, GameAction::Left);
                bool right = isActionPressed(world.inputManager, PlayerIndex::P2, GameAction::Right);
                if (up)    p2Dir.y -= 1.0f;
                if (down)  p2Dir.y += 1.0f;
                if (left)  p2Dir.x -= 1.0f;
                if (right) p2Dir.x += 1.0f;
                if (float l = std::sqrt(p2Dir.x * p2Dir.x + p2Dir.y * p2Dir.y); l > 0.0f) {
                    p2Dir.x /= l; p2Dir.y /= l;
                }
            }

            auto& p2In = world.inputs.get(world.player2);
            p2In.moveDir = p2Dir;

            static bool lastP2Atk = false, lastP2Dash = false, lastP2Bomb = false;
            static float p2BombCD = 0.0f;

            // P2 动作：通过 InputManager 查询（支持改键）
            bool p2Atk = isActionPressed(world.inputManager, PlayerIndex::P2, GameAction::Attack);
            if (p2Atk && !lastP2Atk) { p2In.pendingIntent = ActionIntent::Attack; p2In.intentTimer = 0.2f; }
            lastP2Atk = p2Atk;

            bool p2Dash = isActionPressed(world.inputManager, PlayerIndex::P2, GameAction::Dash);
            if (p2Dash && !lastP2Dash) { p2In.pendingIntent = ActionIntent::Dash; p2In.intentTimer = 0.2f; }
            lastP2Dash = p2Dash;

            if (isActionPressed(world.inputManager, PlayerIndex::P2, GameAction::Jump) && world.zTransforms.has(world.player2)) {
                auto& zc = world.zTransforms.get(world.player2);
                if (zc.isGrounded()) { zc.vz = 800.0f; }
            }

            p2BombCD = std::max(0.0f, p2BombCD - realDt);
            bool p2Bomb = isActionPressed(world.inputManager, PlayerIndex::P2, GameAction::DropBomb);
            if (p2Bomb && !lastP2Bomb && p2BombCD <= 0.0f) {
factory.spawnThrowableBomb(world, world.player2, world.inputs.get(world.player2));
                p2BombCD = 0.5f;
            }
            lastP2Bomb = p2Bomb;

            // P2 暂停/帧步进
            static bool lastP2Pause = false, lastP2Step = false;
            bool p2Pause = isActionPressed(world.inputManager, PlayerIndex::P2, GameAction::Pause);
            if (p2Pause && !lastP2Pause) {
                timeScale = (timeScale == 0.0f) ? 1.0f : 0.0f;
                std::cout << "[TimeScale] " << (timeScale == 0.0f ? "Paused" : "Running") << "\n";
            }
            lastP2Pause = p2Pause;

            bool p2Step = isActionPressed(world.inputManager, PlayerIndex::P2, GameAction::FrameStep);
            if (p2Step && !lastP2Step) { frameStep = true; timeScale = 1.0f; }
            lastP2Step = p2Step;
        }

        // ================================================================
        // ✅ Fixed Timestep 物理循环
        // ================================================================
        while (timeSinceLastUpdate >= TIME_PER_FRAME) {
            timeSinceLastUpdate -= TIME_PER_FRAME;
            float fixedDt = TIME_PER_FRAME.asSeconds();

            if (frameStep) {
                frameStep = false;
                timeScale = 0.0f;
                std::cout << "[FrameStep] Single frame executed\n";
            }

            stateSystem.update(world, fixedDt);
            dashSystem.update(world, fixedDt);
            locomotionSystem.update(world, fixedDt);
            magnetSystem.update(world, fixedDt);
            bombSystem.update(world, fixedDt);

            // Z 轴物理
            for (Entity entity : world.zTransforms.entityList()) {
                if (world.zTransforms.has(entity)) {
                    auto& zTrans = world.zTransforms.get(entity);
                    zTrans.applyGravity(fixedDt);
                    if (zTrans.z <= 0.0f) {
                        zTrans.z = 0.0f;
                        if (world.states.has(entity)) {
                            auto& state = world.states.get(entity);
                            if (state.currentState == CharacterState::KnockedAirborne) {
                                state.currentState = CharacterState::Idle;
                                state.previousState = CharacterState::Idle;
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

            movementSystem.update(world, fixedDt);
            physicalCollisionSystem.update(world, fixedDt);
            attachmentSystem.update(world, fixedDt);
            attackSystem.update(world, fixedDt);
            collisionSystem.update(world, fixedDt);
            damageSystem.update(world, fixedDt);
            lootSpawnSystem.update(world, fixedDt);
            deathSystem.update(world, fixedDt);
            pickupSystem.update(world, fixedDt);

            // PvP 复活
            static float respawn1 = 0.0f, respawn2 = 0.0f;
            if (world.states.has(world.player1) && world.states.get(world.player1).currentState == CharacterState::Dead) {
                respawn1 += fixedDt;
                if (respawn1 >= 2.0f) {
                    world.characters.get(world.player1).currentHP = world.characters.get(world.player1).maxHP;
                    world.states.get(world.player1).currentState = CharacterState::Idle;
                    world.states.get(world.player1).previousState = CharacterState::Idle;
                    world.states.get(world.player1).stateTimer = 0.0f;
                    world.transforms.get(world.player1).position = {250.0f, 300.0f};
                    world.transforms.get(world.player1).velocity = {0.0f, 0.0f};
                    respawn1 = 0.0f;
                }
            }
            if (world.states.has(world.player2) && world.states.get(world.player2).currentState == CharacterState::Dead) {
                respawn2 += fixedDt;
                if (respawn2 >= 2.0f) {
                    world.characters.get(world.player2).currentHP = world.characters.get(world.player2).maxHP;
                    world.states.get(world.player2).currentState = CharacterState::Idle;
                    world.states.get(world.player2).previousState = CharacterState::Idle;
                    world.states.get(world.player2).stateTimer = 0.0f;
                    world.transforms.get(world.player2).position = {750.0f, 300.0f};
                    world.transforms.get(world.player2).velocity = {0.0f, 0.0f};
                    respawn2 = 0.0f;
                }
            }
        }

        cleanupSystem.update(world, TIME_PER_FRAME.asSeconds());

        // --- 渲染 ---
        renderSystem.update(world, window, font, TIME_PER_FRAME.asSeconds(),
            /* colorPlayer2 */ sf::Color(255, 165, 0)); // P2 琥珀色，区别于红色敌人

        // 切回默认 view（ImGui 需要像素坐标）
        window.setView(window.getDefaultView());

        // ========== ImGui 渲染 ==========
        static int renderCounter = 0;
        if (renderCounter++ % 120 == 0) {
            std::cout << "[ImGui-Render] Calling ImGui::SFML::Render (showPanel=" << (showImGuiPanel ? "true" : "false")
                      << ")" << std::endl;
        }
        ImGui::SFML::Render(window);

        window.display();
    }

    // 退出前保存配置
    world.inputManager.saveConfig();

    ImGui::SFML::Shutdown();

    return 0;
}

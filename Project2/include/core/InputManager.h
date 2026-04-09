/**
 * @file InputManager.h
 * @brief 核心输入管理器 - 与平台无关的按键绑定系统
 * 
 * 绝对禁令：本文件禁止包含任何图形库头文件（如 <SFML/...>）
 * 仅允许包含：<unordered_map>, <string>, <fstream>, <nlohmann/json.hpp>
 */

#pragma once

#include <unordered_map>
#include <string>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

/**
 * @brief 游戏逻辑动作枚举（与硬件按键解耦）
 */
enum class GameAction {
    Up,
    Down,
    Left,
    Right,
    Attack,
    Dash,
    DropBomb,
    ToggleHelp,
    Jump,
    Pause,
    FrameStep,
    SpawnDummy  // 新增：生成假人（原鼠标右键）
};

/**
 * @brief 引擎级按键枚举（平台无关的抽象按键）
 */
enum class EngineKey {
    Unknown,
    // 字母键（完整 A-Z）
    A, B, C, D, E, F, G, H, I, J, K, L, M,
    N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
    // 功能键
    Space,
    Escape,
    F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,
    Enter,
    // 修饰键
    LeftShift, RightShift,
    LeftCtrl, RightCtrl,
    LeftAlt, RightAlt,
    // 符号键
    Comma, Period, Slash,
    SemiColon, Quote,
    LeftBracket, RightBracket,
    Backslash,
    Minus, Equal,
    Backquote,
    // 编辑键
    Delete, Insert, Home, End, PageUp, PageDown,
    // 方向键
    Up, Down, Left, Right,
    // 数字键（小键盘）
    Num0, Num1, Num2, Num3, Num4, Num5, Num6, Num7, Num8, Num9,
    // 鼠标按键
    MouseLeft, MouseRight, MouseMiddle,
    MouseX1, MouseX2,
    Max
};

/**
 * @brief 输入管理器 - 处理按键绑定与 JSON 持久化
 */
class InputManager {
public:
    std::unordered_map<GameAction, EngineKey> bindings;
    std::string configPath = "keybinds.json";

    InputManager() {
        // 默认键位初始化
        bindings[GameAction::Up] = EngineKey::W;
        bindings[GameAction::Down] = EngineKey::S;
        bindings[GameAction::Left] = EngineKey::A;
        bindings[GameAction::Right] = EngineKey::D;
        bindings[GameAction::Attack] = EngineKey::J;
        bindings[GameAction::Dash] = EngineKey::Space;
        bindings[GameAction::DropBomb] = EngineKey::G;
        bindings[GameAction::ToggleHelp] = EngineKey::F1;
        bindings[GameAction::Jump] = EngineKey::K;
        bindings[GameAction::Pause] = EngineKey::P;
        bindings[GameAction::FrameStep] = EngineKey::RightBracket;
        bindings[GameAction::SpawnDummy] = EngineKey::MouseRight;
        
        loadConfig(); // 尝试加载用户配置覆盖默认值
    }

    /**
     * @brief 获取动作绑定的引擎按键
     */
    EngineKey getMappedKey(GameAction action) const {
        auto it = bindings.find(action);
        if (it != bindings.end()) {
            return it->second;
        }
        return EngineKey::Unknown;
    }

    /**
     * @brief 将引擎按键转换为人类可读字符串
     */
    std::string keyToString(EngineKey key) const {
        switch (key) {
            case EngineKey::Unknown: return "Unknown";
            // 字母键 A-Z
            case EngineKey::A: return "A";
            case EngineKey::B: return "B";
            case EngineKey::C: return "C";
            case EngineKey::D: return "D";
            case EngineKey::E: return "E";
            case EngineKey::F: return "F";
            case EngineKey::G: return "G";
            case EngineKey::H: return "H";
            case EngineKey::I: return "I";
            case EngineKey::J: return "J";
            case EngineKey::K: return "K";
            case EngineKey::L: return "L";
            case EngineKey::M: return "M";
            case EngineKey::N: return "N";
            case EngineKey::O: return "O";
            case EngineKey::P: return "P";
            case EngineKey::Q: return "Q";
            case EngineKey::R: return "R";
            case EngineKey::S: return "S";
            case EngineKey::T: return "T";
            case EngineKey::U: return "U";
            case EngineKey::V: return "V";
            case EngineKey::W: return "W";
            case EngineKey::X: return "X";
            case EngineKey::Y: return "Y";
            case EngineKey::Z: return "Z";
            // 功能键
            case EngineKey::Space: return "Space";
            case EngineKey::Escape: return "Escape";
            case EngineKey::F1: return "F1";
            case EngineKey::F2: return "F2";
            case EngineKey::F3: return "F3";
            case EngineKey::F4: return "F4";
            case EngineKey::F5: return "F5";
            case EngineKey::F6: return "F6";
            case EngineKey::F7: return "F7";
            case EngineKey::F8: return "F8";
            case EngineKey::F9: return "F9";
            case EngineKey::F10: return "F10";
            case EngineKey::F11: return "F11";
            case EngineKey::F12: return "F12";
            case EngineKey::Enter: return "Enter";
            // 修饰键
            case EngineKey::LeftShift: return "LeftShift";
            case EngineKey::RightShift: return "RightShift";
            case EngineKey::LeftCtrl: return "LeftCtrl";
            case EngineKey::RightCtrl: return "RightCtrl";
            case EngineKey::LeftAlt: return "LeftAlt";
            case EngineKey::RightAlt: return "RightAlt";
            // 符号键
            case EngineKey::Comma: return "Comma";
            case EngineKey::Period: return "Period";
            case EngineKey::Slash: return "Slash";
            case EngineKey::SemiColon: return "SemiColon";
            case EngineKey::Quote: return "Quote";
            case EngineKey::LeftBracket: return "LBracket";
            case EngineKey::RightBracket: return "RBracket";
            case EngineKey::Backslash: return "Backslash";
            case EngineKey::Minus: return "Minus";
            case EngineKey::Equal: return "Equal";
            case EngineKey::Backquote: return "Backquote";
            // 编辑键
            case EngineKey::Delete: return "Delete";
            case EngineKey::Insert: return "Insert";
            case EngineKey::Home: return "Home";
            case EngineKey::End: return "End";
            case EngineKey::PageUp: return "PageUp";
            case EngineKey::PageDown: return "PageDown";
            // 方向键
            case EngineKey::Up: return "Up";
            case EngineKey::Down: return "Down";
            case EngineKey::Left: return "Left";
            case EngineKey::Right: return "Right";
            // 数字键
            case EngineKey::Num0: return "Num0";
            case EngineKey::Num1: return "Num1";
            case EngineKey::Num2: return "Num2";
            case EngineKey::Num3: return "Num3";
            case EngineKey::Num4: return "Num4";
            case EngineKey::Num5: return "Num5";
            case EngineKey::Num6: return "Num6";
            case EngineKey::Num7: return "Num7";
            case EngineKey::Num8: return "Num8";
            case EngineKey::Num9: return "Num9";
            // 鼠标按键
            case EngineKey::MouseLeft: return "MouseLeft";
            case EngineKey::MouseRight: return "MouseRight";
            case EngineKey::MouseMiddle: return "MouseMiddle";
            case EngineKey::MouseX1: return "MouseX1";
            case EngineKey::MouseX2: return "MouseX2";
            default: return "Unknown";
        }
    }

    /**
     * @brief 将字符串转换为引擎按键（用于 JSON 加载）
     */
    EngineKey stringToKey(const std::string& str) const {
        // 字母键 A-Z
        if (str == "A") return EngineKey::A;
        if (str == "B") return EngineKey::B;
        if (str == "C") return EngineKey::C;
        if (str == "D") return EngineKey::D;
        if (str == "E") return EngineKey::E;
        if (str == "F") return EngineKey::F;
        if (str == "G") return EngineKey::G;
        if (str == "H") return EngineKey::H;
        if (str == "I") return EngineKey::I;
        if (str == "J") return EngineKey::J;
        if (str == "K") return EngineKey::K;
        if (str == "L") return EngineKey::L;
        if (str == "M") return EngineKey::M;
        if (str == "N") return EngineKey::N;
        if (str == "O") return EngineKey::O;
        if (str == "P") return EngineKey::P;
        if (str == "Q") return EngineKey::Q;
        if (str == "R") return EngineKey::R;
        if (str == "S") return EngineKey::S;
        if (str == "T") return EngineKey::T;
        if (str == "U") return EngineKey::U;
        if (str == "V") return EngineKey::V;
        if (str == "W") return EngineKey::W;
        if (str == "X") return EngineKey::X;
        if (str == "Y") return EngineKey::Y;
        if (str == "Z") return EngineKey::Z;
        // 功能键
        if (str == "Space") return EngineKey::Space;
        if (str == "Escape") return EngineKey::Escape;
        if (str == "F1") return EngineKey::F1;
        if (str == "F2") return EngineKey::F2;
        if (str == "F3") return EngineKey::F3;
        if (str == "F4") return EngineKey::F4;
        if (str == "F5") return EngineKey::F5;
        if (str == "F6") return EngineKey::F6;
        if (str == "F7") return EngineKey::F7;
        if (str == "F8") return EngineKey::F8;
        if (str == "F9") return EngineKey::F9;
        if (str == "F10") return EngineKey::F10;
        if (str == "F11") return EngineKey::F11;
        if (str == "F12") return EngineKey::F12;
        if (str == "Enter") return EngineKey::Enter;
        // 修饰键
        if (str == "LeftShift") return EngineKey::LeftShift;
        if (str == "RightShift") return EngineKey::RightShift;
        if (str == "LeftCtrl") return EngineKey::LeftCtrl;
        if (str == "RightCtrl") return EngineKey::RightCtrl;
        if (str == "LeftAlt") return EngineKey::LeftAlt;
        if (str == "RightAlt") return EngineKey::RightAlt;
        // 符号键
        if (str == "Comma") return EngineKey::Comma;
        if (str == "Period") return EngineKey::Period;
        if (str == "Slash") return EngineKey::Slash;
        if (str == "SemiColon") return EngineKey::SemiColon;
        if (str == "Quote") return EngineKey::Quote;
        if (str == "LeftBracket") return EngineKey::LeftBracket;
        if (str == "RightBracket") return EngineKey::RightBracket;
        if (str == "LBracket") return EngineKey::LeftBracket;
        if (str == "RBracket") return EngineKey::RightBracket;
        if (str == "Backslash") return EngineKey::Backslash;
        if (str == "Minus") return EngineKey::Minus;
        if (str == "Equal") return EngineKey::Equal;
        if (str == "Backquote") return EngineKey::Backquote;
        // 编辑键
        if (str == "Delete") return EngineKey::Delete;
        if (str == "Insert") return EngineKey::Insert;
        if (str == "Home") return EngineKey::Home;
        if (str == "End") return EngineKey::End;
        if (str == "PageUp") return EngineKey::PageUp;
        if (str == "PageDown") return EngineKey::PageDown;
        // 方向键
        if (str == "Up") return EngineKey::Up;
        if (str == "Down") return EngineKey::Down;
        if (str == "Left") return EngineKey::Left;
        if (str == "Right") return EngineKey::Right;
        // 数字键
        if (str == "Num0") return EngineKey::Num0;
        if (str == "Num1") return EngineKey::Num1;
        if (str == "Num2") return EngineKey::Num2;
        if (str == "Num3") return EngineKey::Num3;
        if (str == "Num4") return EngineKey::Num4;
        if (str == "Num5") return EngineKey::Num5;
        if (str == "Num6") return EngineKey::Num6;
        if (str == "Num7") return EngineKey::Num7;
        if (str == "Num8") return EngineKey::Num8;
        if (str == "Num9") return EngineKey::Num9;
        // 鼠标按键
        if (str == "MouseLeft") return EngineKey::MouseLeft;
        if (str == "MouseRight") return EngineKey::MouseRight;
        if (str == "MouseMiddle") return EngineKey::MouseMiddle;
        if (str == "MouseX1") return EngineKey::MouseX1;
        if (str == "MouseX2") return EngineKey::MouseX2;
        return EngineKey::Unknown;
    }

    /**
     * @brief 将 GameAction 转换为字符串键（用于 JSON）
     */
    std::string actionToString(GameAction action) const {
        switch (action) {
            case GameAction::Up: return "Up";
            case GameAction::Down: return "Down";
            case GameAction::Left: return "Left";
            case GameAction::Right: return "Right";
            case GameAction::Attack: return "Attack";
            case GameAction::Dash: return "Dash";
            case GameAction::DropBomb: return "DropBomb";
            case GameAction::ToggleHelp: return "ToggleHelp";
            case GameAction::Jump: return "Jump";
            case GameAction::Pause: return "Pause";
            case GameAction::FrameStep: return "FrameStep";
            case GameAction::SpawnDummy: return "SpawnDummy";
            default: return "Unknown";
        }
    }

    /**
     * @brief 将字符串转换为 GameAction（用于 JSON 加载）
     */
    GameAction stringToAction(const std::string& str) const {
        if (str == "Up") return GameAction::Up;
        if (str == "Down") return GameAction::Down;
        if (str == "Left") return GameAction::Left;
        if (str == "Right") return GameAction::Right;
        if (str == "Attack") return GameAction::Attack;
        if (str == "Dash") return GameAction::Dash;
        if (str == "DropBomb") return GameAction::DropBomb;
        if (str == "ToggleHelp") return GameAction::ToggleHelp;
        if (str == "Jump") return GameAction::Jump;
        if (str == "Pause") return GameAction::Pause;
        if (str == "FrameStep") return GameAction::FrameStep;
        if (str == "SpawnDummy") return GameAction::SpawnDummy;
        return GameAction::Up; // default
    }

    /**
     * @brief 保存按键绑定到 JSON 文件
     */
    void saveConfig() {
        nlohmann::json json;
        for (const auto& [action, key] : bindings) {
            json[actionToString(action)] = keyToString(key);
        }
        
        std::ofstream file(configPath);
        if (file.is_open()) {
            file << json.dump(4); // 4 空格缩进
            file.close();
            std::cout << "[InputManager] Config saved to " << configPath << std::endl;
        } else {
            std::cerr << "[InputManager] Failed to save config to " << configPath << std::endl;
        }
    }

    /**
     * @brief 从 JSON 文件加载按键绑定
     */
    void loadConfig() {
        std::ifstream file(configPath);
        if (file.is_open()) {
            try {
                nlohmann::json json;
                file >> json;
                
                for (auto& [actionStr, keyStr] : json.items()) {
                    GameAction action = stringToAction(actionStr);
                    EngineKey key = stringToKey(keyStr.get<std::string>());
                    if (key != EngineKey::Unknown) {
                        bindings[action] = key;
                    }
                }
                
                std::cout << "[InputManager] Config loaded from " << configPath << std::endl;
            } catch (const std::exception& e) {
                std::cerr << "[InputManager] Failed to parse config: " << e.what() << std::endl;
            }
            file.close();
        } else {
            std::cout << "[InputManager] No config file found, using defaults" << std::endl;
        }
    }
};

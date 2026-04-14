/**
 * @file InputManager.h
 * @brief 核心输入管理器 - 双玩家独立配置 + JSON 本地持久化
 *
 * 绝对禁令：本文件禁止包含任何图形库头文件（如 <SFML/...>）
 * 仅允许包含：<unordered_map>, <string>, <fstream>, <nlohmann/json.hpp>
 */

#pragma once

#include <unordered_map>
#include <string>
#include <fstream>
#include <iostream>
#include <optional>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

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
    SpawnDummy
};

/**
 * @brief 玩家索引
 */
enum class PlayerIndex {
    P1,
    P2
};

/**
 * @brief 引擎级按键枚举（平台无关的抽象按键）
 */
enum class EngineKey {
    Unknown,
    A, B, C, D, E, F, G, H, I, J, K, L, M,
    N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
    Space,
    Escape,
    Backspace,
    F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,
    Enter,
    LeftShift, RightShift,
    LeftCtrl, RightCtrl,
    LeftAlt, RightAlt,
    Comma, Period, Slash,
    SemiColon, Quote,
    LeftBracket, RightBracket,
    Backslash,
    Minus, Equal,
    Backquote,
    Delete, Insert, Home, End, PageUp, PageDown,
    Up, Down, Left, Right,
    Num0, Num1, Num2, Num3, Num4, Num5, Num6, Num7, Num8, Num9,
    MouseLeft, MouseRight, MouseMiddle,
    MouseX1, MouseX2,
    JoyBtn0, JoyBtn1, JoyBtn2, JoyBtn3, JoyBtn4, JoyBtn5, JoyBtn6, JoyBtn7,
    JoyBtn8, JoyBtn9, JoyBtn10, JoyBtn11, JoyBtn12, JoyBtn13, JoyBtn14, JoyBtn15,
    Max
};

/**
 * @brief 输入管理器 - 双玩家独立绑定 + JSON 持久化
 *
 * 三级优先级加载：user_keybinds.json > data/default_keybinds.json > 硬编码 fallback
 * 每个玩家拥有独立的 GameAction → EngineKey 绑定表。
 */
class InputManager {
public:
    // 双玩家独立绑定表
    std::unordered_map<PlayerIndex, std::unordered_map<GameAction, EngineKey>> bindings;
    std::string userConfigPath = "data/user_keybinds.json";
    std::string defaultConfigPath = "data/default_keybinds.json";

    InputManager() {
        // 三级优先级加载
        if (loadFromJson(userConfigPath)) {
            std::cout << "[InputManager] User config loaded from " << userConfigPath << std::endl;
        } else if (loadFromJson(defaultConfigPath)) {
            std::cout << "[InputManager] Default config loaded from " << defaultConfigPath << std::endl;
            saveConfig();  // 首次运行：复制一份默认配置到 user_keybinds.json
        } else {
            initDefaults();
            std::cout << "[InputManager] Using hardcoded defaults (no JSON found)" << std::endl;
            saveConfig();  // 生成 user_keybinds.json
        }
    }

    /**
     * @brief 初始化 P1/P2 默认硬编码键位
     */
    void initDefaults() {
        // ===== P1 默认：WASD + 字母区 =====
        auto& p1 = bindings[PlayerIndex::P1];
        p1[GameAction::Up]         = EngineKey::W;
        p1[GameAction::Down]       = EngineKey::S;
        p1[GameAction::Left]       = EngineKey::A;
        p1[GameAction::Right]      = EngineKey::D;
        p1[GameAction::Attack]     = EngineKey::J;
        p1[GameAction::Dash]       = EngineKey::Space;
        p1[GameAction::DropBomb]   = EngineKey::G;
        p1[GameAction::Jump]       = EngineKey::K;
        p1[GameAction::ToggleHelp] = EngineKey::Unknown;  // F1 保留给 ImGui 调试面板
        p1[GameAction::Pause]      = EngineKey::P;
        p1[GameAction::FrameStep]  = EngineKey::RightBracket;
        p1[GameAction::SpawnDummy] = EngineKey::MouseRight;

        // ===== P2 默认：方向键 + 小键盘 =====
        auto& p2 = bindings[PlayerIndex::P2];
        p2[GameAction::Up]         = EngineKey::Up;
        p2[GameAction::Down]       = EngineKey::Down;
        p2[GameAction::Left]       = EngineKey::Left;
        p2[GameAction::Right]      = EngineKey::Right;
        p2[GameAction::Attack]     = EngineKey::Num1;
        p2[GameAction::Dash]       = EngineKey::Num0;
        p2[GameAction::DropBomb]   = EngineKey::Num3;
        p2[GameAction::Jump]       = EngineKey::Num2;
        p2[GameAction::ToggleHelp] = EngineKey::Unknown;  // F1 保留给 ImGui 调试面板
        p2[GameAction::Pause]      = EngineKey::Num7;
        p2[GameAction::FrameStep]  = EngineKey::Num9;
        p2[GameAction::SpawnDummy] = EngineKey::MouseRight;
    }

    /**
     * @brief 从指定 JSON 文件加载键位配置
     * @param path JSON 文件路径
     * @return 成功返回 true，失败返回 false
     */
    bool loadFromJson(const std::string& path) {
        std::ifstream file(path);
        if (!file.is_open()) return false;

        try {
            json root;
            file >> root;

            for (auto& [playerKey, pJson] : root.items()) {
                PlayerIndex player = (playerKey == "P1") ? PlayerIndex::P1 : PlayerIndex::P2;
                if (!pJson.is_object()) continue;

                for (auto& [actionStr, keyStr] : pJson.items()) {
                    GameAction action = stringToAction(actionStr);
                    EngineKey key = stringToKey(keyStr.get<std::string>());
                    if (key != EngineKey::Unknown) {
                        bindings[player][action] = key;
                    }
                }
            }
            return true;
        } catch (...) {
            return false;
        }
    }

    // ===== 获取绑定 =====

    EngineKey getMappedKey(PlayerIndex player, GameAction action) const {
        auto pit = bindings.find(player);
        if (pit == bindings.end()) return EngineKey::Unknown;
        auto it = pit->second.find(action);
        if (it == pit->second.end()) return EngineKey::Unknown;
        return it->second;
    }

    void setMappedKey(PlayerIndex player, GameAction action, EngineKey key) {
        bindings[player][action] = key;
    }

    // ===== 字符串转换 =====

    std::string keyToString(EngineKey key) const {
        switch (key) {
            case EngineKey::A: return "A";  case EngineKey::B: return "B";
            case EngineKey::C: return "C";  case EngineKey::D: return "D";
            case EngineKey::E: return "E";  case EngineKey::F: return "F";
            case EngineKey::G: return "G";  case EngineKey::H: return "H";
            case EngineKey::I: return "I";  case EngineKey::J: return "J";
            case EngineKey::K: return "K";  case EngineKey::L: return "L";
            case EngineKey::M: return "M";  case EngineKey::N: return "N";
            case EngineKey::O: return "O";  case EngineKey::P: return "P";
            case EngineKey::Q: return "Q";  case EngineKey::R: return "R";
            case EngineKey::S: return "S";  case EngineKey::T: return "T";
            case EngineKey::U: return "U";  case EngineKey::V: return "V";
            case EngineKey::W: return "W";  case EngineKey::X: return "X";
            case EngineKey::Y: return "Y";  case EngineKey::Z: return "Z";
            case EngineKey::Space: return "Space";
            case EngineKey::Escape: return "Escape";
            case EngineKey::Backspace: return "Backspace";
            case EngineKey::F1: return "F1";  case EngineKey::F2: return "F2";
            case EngineKey::F3: return "F3";  case EngineKey::F4: return "F4";
            case EngineKey::F5: return "F5";  case EngineKey::F6: return "F6";
            case EngineKey::F7: return "F7";  case EngineKey::F8: return "F8";
            case EngineKey::F9: return "F9";  case EngineKey::F10: return "F10";
            case EngineKey::F11: return "F11"; case EngineKey::F12: return "F12";
            case EngineKey::Enter: return "Enter";
            case EngineKey::LeftShift: return "LeftShift";
            case EngineKey::RightShift: return "RightShift";
            case EngineKey::LeftCtrl: return "LeftCtrl";
            case EngineKey::RightCtrl: return "RightCtrl";
            case EngineKey::LeftAlt: return "LeftAlt";
            case EngineKey::RightAlt: return "RightAlt";
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
            case EngineKey::Delete: return "Delete";
            case EngineKey::Insert: return "Insert";
            case EngineKey::Home: return "Home";
            case EngineKey::End: return "End";
            case EngineKey::PageUp: return "PageUp";
            case EngineKey::PageDown: return "PageDown";
            case EngineKey::Up: return "Up";
            case EngineKey::Down: return "Down";
            case EngineKey::Left: return "Left";
            case EngineKey::Right: return "Right";
            case EngineKey::Num0: return "Num0";  case EngineKey::Num1: return "Num1";
            case EngineKey::Num2: return "Num2";  case EngineKey::Num3: return "Num3";
            case EngineKey::Num4: return "Num4";  case EngineKey::Num5: return "Num5";
            case EngineKey::Num6: return "Num6";  case EngineKey::Num7: return "Num7";
            case EngineKey::Num8: return "Num8";  case EngineKey::Num9: return "Num9";
            case EngineKey::MouseLeft: return "MouseLeft";
            case EngineKey::MouseRight: return "MouseRight";
            case EngineKey::MouseMiddle: return "MouseMiddle";
            case EngineKey::MouseX1: return "MouseX1";
            case EngineKey::MouseX2: return "MouseX2";
            case EngineKey::JoyBtn0: return "JoyBtn0";  case EngineKey::JoyBtn1: return "JoyBtn1";
            case EngineKey::JoyBtn2: return "JoyBtn2";  case EngineKey::JoyBtn3: return "JoyBtn3";
            case EngineKey::JoyBtn4: return "JoyBtn4";  case EngineKey::JoyBtn5: return "JoyBtn5";
            case EngineKey::JoyBtn6: return "JoyBtn6";  case EngineKey::JoyBtn7: return "JoyBtn7";
            case EngineKey::JoyBtn8: return "JoyBtn8";  case EngineKey::JoyBtn9: return "JoyBtn9";
            case EngineKey::JoyBtn10: return "JoyBtn10"; case EngineKey::JoyBtn11: return "JoyBtn11";
            case EngineKey::JoyBtn12: return "JoyBtn12"; case EngineKey::JoyBtn13: return "JoyBtn13";
            case EngineKey::JoyBtn14: return "JoyBtn14"; case EngineKey::JoyBtn15: return "JoyBtn15";
            default: return "Unknown";
        }
    }

    EngineKey stringToKey(const std::string& str) const {
        if (str == "A") return EngineKey::A;  if (str == "B") return EngineKey::B;
        if (str == "C") return EngineKey::C;  if (str == "D") return EngineKey::D;
        if (str == "E") return EngineKey::E;  if (str == "F") return EngineKey::F;
        if (str == "G") return EngineKey::G;  if (str == "H") return EngineKey::H;
        if (str == "I") return EngineKey::I;  if (str == "J") return EngineKey::J;
        if (str == "K") return EngineKey::K;  if (str == "L") return EngineKey::L;
        if (str == "M") return EngineKey::M;  if (str == "N") return EngineKey::N;
        if (str == "O") return EngineKey::O;  if (str == "P") return EngineKey::P;
        if (str == "Q") return EngineKey::Q;  if (str == "R") return EngineKey::R;
        if (str == "S") return EngineKey::S;  if (str == "T") return EngineKey::T;
        if (str == "U") return EngineKey::U;  if (str == "V") return EngineKey::V;
        if (str == "W") return EngineKey::W;  if (str == "X") return EngineKey::X;
        if (str == "Y") return EngineKey::Y;  if (str == "Z") return EngineKey::Z;
        if (str == "Space") return EngineKey::Space;
        if (str == "Escape") return EngineKey::Escape;
        if (str == "Backspace") return EngineKey::Backspace;
        if (str == "F1") return EngineKey::F1;  if (str == "F2") return EngineKey::F2;
        if (str == "F3") return EngineKey::F3;  if (str == "F4") return EngineKey::F4;
        if (str == "F5") return EngineKey::F5;  if (str == "F6") return EngineKey::F6;
        if (str == "F7") return EngineKey::F7;  if (str == "F8") return EngineKey::F8;
        if (str == "F9") return EngineKey::F9;  if (str == "F10") return EngineKey::F10;
        if (str == "F11") return EngineKey::F11; if (str == "F12") return EngineKey::F12;
        if (str == "Enter") return EngineKey::Enter;
        if (str == "LeftShift") return EngineKey::LeftShift;
        if (str == "RightShift") return EngineKey::RightShift;
        if (str == "LeftCtrl") return EngineKey::LeftCtrl;
        if (str == "RightCtrl") return EngineKey::RightCtrl;
        if (str == "LeftAlt") return EngineKey::LeftAlt;
        if (str == "RightAlt") return EngineKey::RightAlt;
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
        if (str == "Delete") return EngineKey::Delete;
        if (str == "Insert") return EngineKey::Insert;
        if (str == "Home") return EngineKey::Home;
        if (str == "End") return EngineKey::End;
        if (str == "PageUp") return EngineKey::PageUp;
        if (str == "PageDown") return EngineKey::PageDown;
        if (str == "Up") return EngineKey::Up;
        if (str == "Down") return EngineKey::Down;
        if (str == "Left") return EngineKey::Left;
        if (str == "Right") return EngineKey::Right;
        if (str == "Num0") return EngineKey::Num0;  if (str == "Num1") return EngineKey::Num1;
        if (str == "Num2") return EngineKey::Num2;  if (str == "Num3") return EngineKey::Num3;
        if (str == "Num4") return EngineKey::Num4;  if (str == "Num5") return EngineKey::Num5;
        if (str == "Num6") return EngineKey::Num6;  if (str == "Num7") return EngineKey::Num7;
        if (str == "Num8") return EngineKey::Num8;  if (str == "Num9") return EngineKey::Num9;
        if (str == "MouseLeft") return EngineKey::MouseLeft;
        if (str == "MouseRight") return EngineKey::MouseRight;
        if (str == "MouseMiddle") return EngineKey::MouseMiddle;
        if (str == "MouseX1") return EngineKey::MouseX1;
        if (str == "MouseX2") return EngineKey::MouseX2;
        if (str == "JoyBtn0") return EngineKey::JoyBtn0;  if (str == "JoyBtn1") return EngineKey::JoyBtn1;
        if (str == "JoyBtn2") return EngineKey::JoyBtn2;  if (str == "JoyBtn3") return EngineKey::JoyBtn3;
        if (str == "JoyBtn4") return EngineKey::JoyBtn4;  if (str == "JoyBtn5") return EngineKey::JoyBtn5;
        if (str == "JoyBtn6") return EngineKey::JoyBtn6;  if (str == "JoyBtn7") return EngineKey::JoyBtn7;
        if (str == "JoyBtn8") return EngineKey::JoyBtn8;  if (str == "JoyBtn9") return EngineKey::JoyBtn9;
        if (str == "JoyBtn10") return EngineKey::JoyBtn10; if (str == "JoyBtn11") return EngineKey::JoyBtn11;
        if (str == "JoyBtn12") return EngineKey::JoyBtn12; if (str == "JoyBtn13") return EngineKey::JoyBtn13;
        if (str == "JoyBtn14") return EngineKey::JoyBtn14; if (str == "JoyBtn15") return EngineKey::JoyBtn15;
        return EngineKey::Unknown;
    }

    std::string actionToString(GameAction action) const {
        switch (action) {
            case GameAction::Up:         return "Move Up";
            case GameAction::Down:       return "Move Down";
            case GameAction::Left:       return "Move Left";
            case GameAction::Right:      return "Move Right";
            case GameAction::Attack:     return "Attack";
            case GameAction::Dash:       return "Dash / Kick";
            case GameAction::DropBomb:   return "Drop Bomb";
            case GameAction::Jump:       return "Jump";
            case GameAction::ToggleHelp: return "Toggle Help";
            case GameAction::Pause:      return "Pause";
            case GameAction::FrameStep:  return "Frame Step";
            case GameAction::SpawnDummy: return "Spawn Dummy";
            default: return "Unknown";
        }
    }

    GameAction stringToAction(const std::string& str) const {
        if (str == "Move Up")       return GameAction::Up;
        if (str == "Move Down")     return GameAction::Down;
        if (str == "Move Left")     return GameAction::Left;
        if (str == "Move Right")    return GameAction::Right;
        if (str == "Attack")        return GameAction::Attack;
        if (str == "Dash / Kick")   return GameAction::Dash;
        if (str == "Dash")          return GameAction::Dash;
        if (str == "Drop Bomb")     return GameAction::DropBomb;
        if (str == "Jump")          return GameAction::Jump;
        if (str == "Toggle Help")   return GameAction::ToggleHelp;
        if (str == "Pause")         return GameAction::Pause;
        if (str == "Frame Step")    return GameAction::FrameStep;
        if (str == "Spawn Dummy")   return GameAction::SpawnDummy;
        return GameAction::Up;
    }

    // ===== 冲突检测 =====

    /**
     * @brief 查找某个 EngineKey 被当前玩家的哪个动作占用
     * @return 如果找到则返回 GameAction，否则返回 std::nullopt
     */
    std::optional<GameAction> findActionByKey(PlayerIndex player, EngineKey key) const {
        auto pit = bindings.find(player);
        if (pit == bindings.end()) return std::nullopt;
        for (auto& [action, boundKey] : pit->second) {
            if (boundKey == key) return action;
        }
        return std::nullopt;
    }

    /**
     * @brief 重置所有绑定为默认值
     */
    void resetToDefaults() {
        // 从默认模板加载，然后保存到用户配置
        if (loadFromJson(defaultConfigPath)) {
            std::cout << "[InputManager] Reset to defaults from " << defaultConfigPath << std::endl;
        } else {
            initDefaults();
            std::cout << "[InputManager] Reset to hardcoded defaults" << std::endl;
        }
        saveConfig();  // 写入 user_keybinds.json
    }

    // ===== JSON 持久化 =====

    /**
     * @brief 保存 P1/P2 绑定到 user_keybinds.json
     *
     * 格式:
     * {
     *   "P1": { "Move Up": "W", "Attack": "J", ... },
     *   "P2": { "Move Up": "Up", "Attack": "Num1", ... }
     * }
     */
    void saveConfig() {
        json root;
        for (auto& [player, playerBindings] : bindings) {
            std::string playerKey = (player == PlayerIndex::P1) ? "P1" : "P2";
            json pJson = json::object();
            for (auto& [action, key] : playerBindings) {
                pJson[actionToString(action)] = keyToString(key);
            }
            root[playerKey] = pJson;
        }

        std::ofstream file(userConfigPath);
        if (file.is_open()) {
            file << root.dump(4);
            file.close();
            std::cout << "[InputManager] Config saved to " << userConfigPath << std::endl;
        } else {
            std::cerr << "[InputManager] Failed to save config to " << userConfigPath << std::endl;
        }
    }
};

#pragma once

/**
 * @brief 游戏状态组件（纯数据）
 * 
 * 存储游戏全局状态
 * 
 * @see docs/01_DATA_SCHEMA.md - 数据字典规范
 * @see GameFlowSystem - 游戏流程系统
 */
struct GameStateComponent {
    int state{0}; // 0=Menu, 1=Playing, 2=Paused, 3=GameOver
    float gameTime{0.0f};       // 游戏内时间（秒）
    int difficultyLevel{1};     // 难度等级
};

#pragma once

/**
 * @brief AI 导演状态组件（纯数据）
 * 
 * AI 导演的全局状态，用于控制随机事件和怪物生成
 * 
 * @see docs/01_DATA_SCHEMA.md - 数据字典规范
 * @see EventDirectorSystem - 事件导演系统
 */
struct DirectorStateComponent {
    float lastSpawnTime{0.0f};  // 上次生成怪物时间
    float spawnCooldown{60.0f}; // 生成冷却（秒）
    int currentWave{0};         // 当前波次
    bool isEventActive{false};  // 是否有活跃事件
};

#pragma once

/**
 * @brief AI 导演状态组件（纯数据）
 *
 * ⚠️ 重构（ECS 纯净原则）：
 * - spawnCooldown / lastSpawnTime → 冷却由 DirectorSystem 内部维护
 * - isEventActive → 描述"未来行为"，移除
 *
 * @see docs/01_DATA_SCHEMA.md - 数据字典规范
 * @see DirectorSystem - 事件导演系统（待实现）
 */
struct DirectorStateComponent {
    int currentWave{0};         // 当前波次
};

#pragma once

/**
 * @brief 炸弹组件（纯数据）
 *
 * ⚠️ 重构（ECS 纯净原则）：
 * - fuseTimer: 引信剩余时间 → 描述"当前剩余多少"，是合法状态
 * - isKicked: 记录"是否被踢飞" → 移除，踢飞状态由 BombSystem 内部维护
 * - lastPosX/Y: 上一帧位置（历史记录）→ 移除，由 BombSystem 内部保存
 *
 * @see BombSystem - 炸弹生命周期管理
 */
struct BombComponent {
    float fuseTimer{3.0f};    // 引信倒计时（秒）
};

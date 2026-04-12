#pragma once

/**
 * @brief 炸弹组件
 * 
 * ⚠️ 架构设计：
 * - 引信倒计时（3 秒爆炸）
 * - 踢飞状态标记
 * - 配合 ZTransformComponent 实现弹跳物理
 * 
 * @see BombSystem - 炸弹系统
 */
struct BombComponent {
    float fuseTimer{3.0f};    // 引信倒计时（秒）
    bool isKicked{false};     // 是否被踢飞
    float lastPosX{0.0f};     // 上一帧位置（用于 CCD 连续碰撞检测）
    float lastPosY{0.0f};
};

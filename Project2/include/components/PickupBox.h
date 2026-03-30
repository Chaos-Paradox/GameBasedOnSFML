#pragma once

/**
 * @brief 拾取碰撞框组件（纯数据）
 * 
 * 独立于战斗 Hurtbox 的拾取碰撞框。
 * 只负责吃道具，不参与战斗计算。
 * 
 * @see PickupSystem - 拾取判定系统
 */
struct PickupBoxComponent {
    float width{30.0f};   // 拾取框宽度
    float height{30.0f};  // 拾取框高度
};

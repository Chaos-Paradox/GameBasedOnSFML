#pragma once

/**
 * @brief 死亡标记组件（延迟销毁）
 * 
 * ⚠️ 架构设计：
 * - 标记实体待销毁，但不立即执行
 * - 由 CleanupSystem 在帧末统一清理
 * - 防止 std::out_of_range 崩溃（游魂访问）
 * 
 * @see CleanupSystem - 清理系统
 */
struct DeathTag {
    // 空结构体，仅作为标记
};

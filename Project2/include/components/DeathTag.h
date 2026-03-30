#pragma once

/**
 * @brief 死亡标签（单帧事件）
 * 
 * 当实体 HP ≤ 0 时挂载，LootSpawnSystem 读取后生成掉落物，
 * 然后 DeathSystem 销毁该实体。
 * 
 * ⚠️ 生命周期：1 帧
 * - DamageSystem 挂载
 * - LootSpawnSystem 读取并生成掉落物
 * - DeathSystem 销毁实体
 * 
 * @see LootSpawnSystem - 掉落生成系统
 * @see DeathSystem - 死亡处理系统
 */
struct DeathTag {
    float timestamp{0.0f};  // 死亡时间（用于调试）
};

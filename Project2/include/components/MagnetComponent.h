#pragma once

/**
 * @brief 磁力吸附组件
 * 
 * ⚠️ 关键设计：挂载在玩家身上，而非掉落物！
 * 
 * 玩家拥有"吸收半径"，掉落物进入半径后自动飞向玩家。
 * 
 * 设计优势：
 * - 逻辑直观：玩家"吸收"掉落物，而非掉落物主动吸附
 * - 易于扩展：可通过装备/道具修改玩家的吸收半径
 * - 性能优化：玩家数量远少于掉落物，遍历更高效
 * 
 * @see MagnetSystem - 磁力吸附系统
 * @see LootSpawnSystem - 掉落生成系统
 */
struct MagnetComponent {
    float magnetRadius{200.0f};  // 吸收半径（像素）默认 200（爽游体验）
    float magnetSpeed{400.0f};   // 吸收速度（像素/秒）默认 400
};

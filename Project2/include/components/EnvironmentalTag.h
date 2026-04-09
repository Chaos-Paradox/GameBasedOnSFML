#pragma once

/**
 * @brief 环境标记 Tag（纯数据）
 * 
 * 当玩家进入特定区域时被临时挂载
 * 
 * @see docs/01_DATA_SCHEMA.md - 数据字典规范
 * @see BiomeZoneComponent - 生物群落区域
 */
struct EnvironmentalTag {
    int biomeType{0}; // 对应 BiomeZoneComponent::BiomeType
    float enterTime{0.0f}; // 进入时间（用于计算适应度）
};

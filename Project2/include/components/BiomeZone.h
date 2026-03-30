#pragma once
#include "../math/Rect.h"

/**
 * @brief 生物群落区域组件（纯数据）
 * 
 * 挂载在地图上不可见的触发器实体上
 * 
 * @see docs/01_DATA_SCHEMA.md - 数据字典规范
 * @see SpatialTriggerSystem - 空间触发器系统
 */
struct BiomeZoneComponent {
    enum class BiomeType : unsigned char {
        Normal,         // 普通区域
        Radiation,      // 辐射区
        ExtremeCold,    // 极寒区
        Toxic,          // 毒气区
        Sanctuary,      // 安全区（禁止战斗）
    };
    
    BiomeType biomeType{BiomeType::Normal};
    Rect bounds; // 区域范围
    
    // 区域效果（可选）
    float damagePerSecond{0.0f}; // 区域伤害/秒
    int damageType{0}; // 0=Physical, 1=Fire, 2=Toxic, 3=Ice
};

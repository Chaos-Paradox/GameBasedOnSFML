#pragma once
#include "../core/Component.h"
#include "../core/ECS.h"
#include "../components/DamageEventComponent.h"
#include "../components/DamageTextComponent.h"
#include <sstream>

/**
 * @brief 伤害飘字生成系统
 * 
 * 职责：
 * - 遍历所有 DamageEventComponent 事件实体
 * - 为每个伤害事件创建一个飘字实体
 * - 挂载 DamageTextComponent
 * 
 * ⚠️ 时序：必须在 DamageSystem 之后，CleanupSystem 之前执行
 * 
 * @see DamageTextComponent - 飘字组件
 * @see DamageTextRenderSystem - 飘字渲染系统
 */
class DamageTextSpawnerSystem {
public:
    void update(
        const ComponentStore<DamageEventComponent>& damageEvents,
        ComponentStore<DamageTextComponent>& damageTexts,
        ECS& ecs)
    {
        auto eventEntities = damageEvents.entityList();
        for (Entity eventEntity : eventEntities) {
            const auto& event = damageEvents.get(eventEntity);
            
            // 安全检查
            if (event.target == INVALID_ENTITY) {
                continue;
            }
            
            // ← 【核心功能】创建飘字实体
            Entity textEntity = createDamageText(ecs, damageTexts, event);
            
            (void)textEntity;  // 飘字实体已创建
        }
    }
    
private:
    /**
     * @brief 创建伤害飘字实体
     * 
     * @param ecs ECS 实例
     * @param damageTexts DamageTextComponent 存储
     * @param event 伤害事件
     * @return 飘字实体 ID
     */
    Entity createDamageText(
        ECS& ecs,
        ComponentStore<DamageTextComponent>& damageTexts,
        const DamageEventComponent& event)
    {
        Entity textEntity = ecs.create();
        
        // ← 将伤害数值转为字符串
        std::ostringstream oss;
        oss << event.actualDamage;
        std::string damageText = oss.str();
        
        // ← 暴击添加感叹号
        if (event.isCritical) {
            damageText += "!";
        }
        
        // ← 挂载 DamageTextComponent
        damageTexts.add(textEntity, {
            .text = damageText,
            .timer = 1.0f,                    // 存活 1 秒
            .position = event.hitPosition,    // 继承打击位置
            .velocity = {0.0f, -50.0f},       // 向上漂浮 50 像素/秒
            .isCritical = event.isCritical,
            .alpha = 1.0f,
            .fontSize = event.isCritical ? 32.0f : 24.0f,  // 暴击字体更大
            .fadeOutStart = 0.5f              // 最后 0.5 秒淡出
        });
        
        return textEntity;
    }
};

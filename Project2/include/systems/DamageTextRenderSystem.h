#pragma once
#include "../core/Component.h"
#include "../core/ECS.h"
#include "../components/DamageTextComponent.h"
#include <SFML/Graphics.hpp>
#include <vector>
#include <cstdint>

/**
 * @brief 伤害飘字渲染系统
 * 
 * 职责：
 * - 遍历所有 DamageTextComponent
 * - 更新位置（向上漂浮）
 * - 更新透明度（淡出效果）
 * - 使用 sf::Text 渲染飘字
 * - 销毁 timer <= 0 的飘字实体
 * 
 * ⚠️ 依赖：需要传入 sf::RenderWindow 和 字体资源
 * 
 * @see DamageTextComponent - 飘字组件
 * @see DamageTextSpawnerSystem - 飘字生成系统
 */
class DamageTextRenderSystem {
public:
    /**
     * @brief 更新并渲染所有飘字
     * 
     * @param damageTexts DamageTextComponent 存储
     * @param window SFML 渲染窗口
     * @param font SFML 字体资源
     * @param dt 帧时间
     * @param ecs ECS 实例（用于销毁实体）
     */
    void update(
        ComponentStore<DamageTextComponent>& damageTexts,
        sf::RenderWindow& window,
        const sf::Font& font,
        ECS& ecs,
        float dt)
    {
        std::vector<Entity> entitiesToDestroy;
        
        auto entities = damageTexts.entityList();
        for (Entity entity : entities) {
            if (!damageTexts.has(entity)) {
                continue;
            }
            
            auto& textComp = damageTexts.get(entity);
            
            // ← 1. 更新计时器
            textComp.timer -= dt;
            
            // ← 2. 更新位置（向上漂浮）
            textComp.position.x += textComp.velocity.x * dt;
            textComp.position.y += textComp.velocity.y * dt;
            
            // ← 3. 更新透明度（淡出效果）
            if (textComp.timer < textComp.fadeOutStart) {
                textComp.alpha = textComp.timer / textComp.fadeOutStart;
            }
            
            // ← 4. 渲染飘字
            renderText(window, font, textComp);
            
            // ← 5. 标记需要销毁的实体
            if (textComp.timer <= 0.0f) {
                entitiesToDestroy.push_back(entity);
            }
        }
        
        // ← 6. 销毁到期的飘字实体
        for (Entity entity : entitiesToDestroy) {
            if (damageTexts.has(entity)) {
                damageTexts.remove(entity);
            }
            ecs.destroy(entity);
        }
    }
    
private:
    /**
     * @brief 渲染单个飘字
     * 
     * @param window SFML 渲染窗口
     * @param font SFML 字体资源
     * @param textComp 飘字组件
     */
    void renderText(
        sf::RenderWindow& window,
        const sf::Font& font,
        const DamageTextComponent& textComp)
    {
        // ← SFML 3：Text 构造函数必须传入 font
        sf::Text text(font, textComp.text, static_cast<unsigned int>(textComp.fontSize));
        
        // ← SFML 3 没有 setBold，使用字体样式
        if (textComp.isCritical) {
            text.setStyle(sf::Text::Style::Bold);
        }
        
        // ← 设置位置（中心对齐）
        auto bounds = text.getLocalBounds();
        text.setOrigin({bounds.size.x / 2.0f, bounds.size.y / 2.0f});
        text.setPosition({textComp.position.x, textComp.position.y});
        
        // ← 根据暴击设置颜色（SFML 3 使用 uint8_t）
        uint8_t alphaValue = static_cast<uint8_t>(textComp.alpha * 255.0f);
        if (textComp.isCritical) {
            // 暴击：红色
            text.setFillColor(sf::Color(255, 50, 50, alphaValue));
        } else {
            // 普通：白色
            text.setFillColor(sf::Color(255, 255, 255, alphaValue));
        }
        
        // ← 添加黑色描边（提高可见度）
        text.setOutlineColor(sf::Color(0, 0, 0, alphaValue));
        text.setOutlineThickness(2.0f);
        
        window.draw(text);
    }
};

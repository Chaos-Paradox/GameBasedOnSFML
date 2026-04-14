#pragma once

#ifdef ENABLE_SFML

#include <SFML/Graphics.hpp>

#include "core/GameWorld.h"
#include "core/Entity.h"
#include "systems/DamageTextRenderSystem.h"
#include <vector>
#include <algorithm>
#include <unordered_map>
#include <cmath>

/**
 * @brief 2.5D 伪立体渲染系统 (Y-Sorting + 分层渲染)
 *
 * ✅ 重构后签名：
 *   void update(GameWorld& world, sf::RenderWindow& window, sf::Font& font, float dt)
 */
class RenderSystem {
private:
    float ENTITY_SIZE_VAL;
    sf::Color COLOR_BACKGROUND_VAL;
    sf::Color COLOR_PLAYER_VAL;
    sf::Color COLOR_ENEMY_VAL;
    sf::Color COLOR_HURT_VAL;
    sf::Color COLOR_DEAD_VAL;
    sf::Color COLOR_LOOT_VAL;

    // 成员变量：DamageTextRenderSystem 实例（避免每帧栈上构造）
    DamageTextRenderSystem m_damageTextSystem;

    // 性能优化：缓存网格（避免每帧创建 40+ sf::VertexArray）
    sf::VertexArray cachedGridLines;
    bool gridInitialized = false;

public:
    RenderSystem(float entitySize = 40.0f,
                sf::Color colorBackground = sf::Color(50, 50, 70),
                sf::Color colorPlayer = sf::Color(100, 200, 255),
                sf::Color colorEnemy = sf::Color(255, 100, 100),
                sf::Color colorHurt = sf::Color(255, 100, 100),
                sf::Color colorDead = sf::Color(100, 100, 100),
                sf::Color colorLOOT = sf::Color(255, 255, 0))
        : ENTITY_SIZE_VAL(entitySize), COLOR_BACKGROUND_VAL(colorBackground),
          COLOR_PLAYER_VAL(colorPlayer),
          COLOR_ENEMY_VAL(colorEnemy), COLOR_HURT_VAL(colorHurt), COLOR_DEAD_VAL(colorDead),
          COLOR_LOOT_VAL(colorLOOT) {}

    void update(
        GameWorld& world,
        sf::RenderWindow& window,
        sf::Font& font,
        float dt,
        // 渲染配置
        sf::Color colorPlayer2 = sf::Color(255, 100, 100),
        float entitySize = 40.0f,
        sf::Color colorBackground = sf::Color(50, 50, 70),
        sf::Color colorPlayer = sf::Color(100, 200, 255),
        sf::Color colorEnemy = sf::Color(255, 100, 100),
        sf::Color colorHurt = sf::Color(255, 100, 100),
        sf::Color colorDead = sf::Color(100, 100, 100),
        sf::Color colorLOOT = sf::Color(255, 255, 0))
    {
        (void)dt;
        const float ENTITY_SIZE = entitySize;
        const sf::Color COLOR_BACKGROUND = colorBackground;
        const sf::Color COLOR_PLAYER = colorPlayer;
        const sf::Color COLOR_ENEMY = colorEnemy;
        const sf::Color COLOR_HURT = colorHurt;
        const sf::Color COLOR_DEAD = colorDead;
        const sf::Color COLOR_LOOT = colorLOOT;
        window.clear(COLOR_BACKGROUND);

        // ========== Camera Shake ==========
        // 获取当前 view（可能是动态相机设置的），而不是用 defaultView 覆盖它
        sf::View view = window.getView();
        sf::Vector2f originalCenter = view.getCenter();
        auto& juice = world.juice;

        if (juice.shakeTimer > 0.0f) {
            juice.shakeTimer -= dt;
            float offsetX = ((std::rand() % 201) / 100.0f - 1.0f) * juice.shakeIntensity;
            float offsetY = ((std::rand() % 201) / 100.0f - 1.0f) * juice.shakeIntensity;
            view.setCenter(sf::Vector2f(originalCenter.x + offsetX, originalCenter.y + offsetY));
            juice.shakeIntensity *= 0.9f;
        }

        window.setView(view);

        // ========== 网格背景 ==========
        renderGrid(window);

        // ========== 围栏 ==========
        renderFence(window, world);

        // ========== 收集所有可渲染实体 ==========
        std::vector<Entity> renderQueue;
        for (Entity entity : world.characters.entityList()) {
            if (world.transforms.has(entity) && world.states.has(entity)) {
                renderQueue.push_back(entity);
            }
        }

        // ========== Y-Sorting (考虑 z 轴，空中的实体往后排) ==========
        std::sort(renderQueue.begin(), renderQueue.end(), [&world](Entity a, Entity b) {
            float ay = world.transforms.get(a).position.y;
            float by = world.transforms.get(b).position.y;
            float az = world.zTransforms.has(a) ? world.zTransforms.get(a).z : 0.0f;
            float bz = world.zTransforms.has(b) ? world.zTransforms.get(b).z : 0.0f;
            // 空中的实体往后排（视觉上更远）
            return (ay - az * 0.5f) < (by - bz * 0.5f);
        });

        // ========== 阴影层 ==========
        for (Entity entity : renderQueue) {
            if (!world.transforms.has(entity)) continue;
            const auto& transform = world.transforms.get(entity);

            const HurtboxComponent* hurtbox = world.hurtboxes.has(entity) ? &world.hurtboxes.get(entity) : nullptr;
            float baseRadius = hurtbox ? hurtbox->radius : 30.0f;

            float zHeight = 0.0f;
            if (world.zTransforms.has(entity)) {
                zHeight = world.zTransforms.get(entity).z;
            }

            float shadowRadius = baseRadius * (1.0f - zHeight * 0.002f);
            if (shadowRadius < baseRadius * 0.3f) shadowRadius = baseRadius * 0.3f;

            int shadowAlpha = 180 - static_cast<int>(zHeight * 1.5f);
            if (shadowAlpha < 40) shadowAlpha = 40;

            sf::CircleShape shadow(shadowRadius);
            shadow.setOrigin(sf::Vector2f(shadowRadius, shadowRadius / 2.0f));
            shadow.setScale(sf::Vector2f(1.0f, 0.5f));
            shadow.setFillColor(sf::Color(0, 0, 0, shadowAlpha));
            shadow.setPosition(sf::Vector2f(transform.position.x, transform.position.y));
            window.draw(shadow);

            if (zHeight > 5.0f) {
                sf::VertexArray heightLine(sf::PrimitiveType::Lines, 2);
                heightLine[0].position = sf::Vector2f(transform.position.x, transform.position.y);
                heightLine[0].color = sf::Color(255, 255, 255, 80);
                heightLine[1].position = sf::Vector2f(transform.position.x, transform.position.y - zHeight);
                heightLine[1].color = sf::Color(255, 255, 255, 20);
                window.draw(heightLine);
            }
        }

        // ========== 实体层 ==========
        for (Entity entity : renderQueue) {
            bool isP1 = (entity == world.player1);
            bool isP2 = (entity == world.player2);
            sf::Color pColor = isP1 ? colorPlayer : (isP2 ? colorPlayer2 : colorEnemy);

            const DashComponent* dashPtr = world.dashes.has(entity) ? &world.dashes.get(entity) : nullptr;
            const ZTransformComponent* zPtr = world.zTransforms.has(entity) ? &world.zTransforms.get(entity) : nullptr;
            const HurtboxComponent* hurtboxPtr = world.hurtboxes.has(entity) ? &world.hurtboxes.get(entity) : nullptr;

            renderEntity(window, world.transforms.get(entity), world.characters.get(entity), world.states.get(entity),
                        isP1 || isP2, dashPtr, zPtr, hurtboxPtr, entitySize, pColor, colorEnemy, colorDead, colorHurt);
        }

        // ========== 调试可视化 ==========
        renderHitboxes(window, world.transforms, world.hitboxes, world.zTransforms);
        renderAttackSectors(window, world.states, world.transforms, world.attackStates, world.zTransforms);
        renderLoot(window, world.transforms, world.itemDatas, world.zTransforms, colorLOOT);
        renderBombs(window, world.transforms, world.bombs, world.zTransforms);

        // ========== UI层 ==========
        window.setView(window.getDefaultView());
        m_damageTextSystem.update(world, window, font, dt);


        // ← 不再调用 window.display()，由主循环统一管理
    }

private:
    void renderEntity(sf::RenderWindow& window, const TransformComponent& trans, const CharacterComponent& chara,
                     const StateMachineComponent& state, bool isPlayer, const DashComponent* dash,
                     const ZTransformComponent* zComp, const HurtboxComponent* hurtbox,
                     float entitySize, sf::Color colorPlayer, sf::Color colorEnemy,
                     sf::Color colorDead, sf::Color colorHurt) {

        float renderX = trans.position.x;
        float renderY = trans.position.y;
        float currentEntitySize = entitySize;

        if (zComp && zComp->z > 0.0f) {
            renderY -= zComp->z;
            float heightScale = 1.0f - zComp->z * 0.001f;
            if (heightScale < 0.7f) heightScale = 0.7f;
            currentEntitySize *= heightScale;
        }

        sf::RectangleShape rect({currentEntitySize, currentEntitySize});
        rect.setOrigin({currentEntitySize / 2.0f, currentEntitySize / 2.0f});
        rect.setPosition({renderX, renderY});

        sf::Color entityColor = isPlayer ? colorPlayer : colorEnemy;

        if (state.currentState == CharacterState::Dead) {
            entityColor = colorDead;
        } else if (state.currentState == CharacterState::Hurt) {
            entityColor = colorHurt;
        } else if (state.currentState == CharacterState::Dash && dash != nullptr) {
            if (dash->isInvincible) {
                entityColor = sf::Color(0, 255, 255, 180);
            } else {
                entityColor = sf::Color(isPlayer ? 50 : 200, 100, 100, 200);
            }
        }

        rect.setFillColor(entityColor);
        window.draw(rect);

        float hpBarWidth = 40.0f;
        float hpBarHeight = 5.0f;
        float hpBarY = renderY - currentEntitySize / 2.0f - 8.0f;

        sf::RectangleShape hpBarBg({hpBarWidth, hpBarHeight});
        hpBarBg.setOrigin({hpBarWidth / 2.0f, hpBarHeight / 2.0f});
        hpBarBg.setPosition({trans.position.x, hpBarY});
        hpBarBg.setFillColor(sf::Color(200, 0, 0));
        window.draw(hpBarBg);

        float hpPercent = static_cast<float>(chara.currentHP) / std::max(1, chara.maxHP);
        sf::RectangleShape hpBarFg({hpBarWidth * hpPercent, hpBarHeight});
        hpBarFg.setOrigin({hpBarWidth / 2.0f, hpBarHeight / 2.0f});
        hpBarFg.setPosition({trans.position.x, hpBarY});
        hpBarFg.setFillColor(sf::Color(0, 200, 0));
        window.draw(hpBarFg);
    }

    void renderHitboxes(sf::RenderWindow& window, const ComponentStore<TransformComponent>& transforms,
                       const ComponentStore<HitboxComponent>& hitboxes,
                       const ComponentStore<ZTransformComponent>& zTransforms) {
        for (Entity entity : hitboxes.entityList()) {
            if (!transforms.has(entity)) continue;
            const auto& transform = transforms.get(entity);
            const auto& hitbox = hitboxes.get(entity);

            float z = zTransforms.has(entity) ? zTransforms.get(entity).z : 0.0f;

            float centerX = transform.position.x + hitbox.offset.x;
            float centerY = transform.position.y + hitbox.offset.y - z;

            sf::CircleShape circle(hitbox.radius);
            circle.setOrigin({hitbox.radius, hitbox.radius});
            circle.setPosition({centerX, centerY});
            circle.setFillColor(sf::Color(255, 255, 0, 128));
            window.draw(circle);
        }
    }

    void renderAttackSectors(
        sf::RenderWindow& window,
        const ComponentStore<StateMachineComponent>& states,
        const ComponentStore<TransformComponent>& transforms,
        const ComponentStore<AttackStateComponent>& attackStates,
        const ComponentStore<ZTransformComponent>& zTransforms)
    {
        for (Entity entity : attackStates.entityList()) {
            if (!states.has(entity) || states.get(entity).currentState != CharacterState::Attack)
                continue;
            if (!transforms.has(entity)) continue;

            const auto& transform = transforms.get(entity);
            const auto& attackState = attackStates.get(entity);

            float baseAngle = std::atan2(transform.facingY, transform.facingX);
            float halfArcRad = (attackState.attackArc / 2.0f) * (M_PI / 180.0f);
            float startAngle = baseAngle - halfArcRad;
            float endAngle = baseAngle + halfArcRad;
            float range = attackState.attackRange;

            const int arcPoints = 32;
            sf::ConvexShape sector;
            sector.setPointCount(2 + arcPoints);
            sector.setPoint(0, sf::Vector2f(0, 0));

            for (int i = 0; i <= arcPoints; ++i) {
                float angle = startAngle + (endAngle - startAngle) * (static_cast<float>(i) / arcPoints);
                float px = std::cos(angle) * range;
                float py = std::sin(angle) * range;
                sector.setPoint(i + 1, sf::Vector2f(px, py));
            }

            float renderY = transform.position.y;
            if (zTransforms.has(entity) && zTransforms.get(entity).z > 0.0f) {
                renderY -= zTransforms.get(entity).z;
            }
            sector.setPosition(sf::Vector2f(transform.position.x, renderY));
            sector.setFillColor(sf::Color(255, 50, 50, 60));
            sector.setOutlineColor(sf::Color(255, 100, 100, 180));
            sector.setOutlineThickness(1.5f);
            window.draw(sector);
        }
    }

    void renderLoot(sf::RenderWindow& window, const ComponentStore<TransformComponent>& transforms,
                   const ComponentStore<ItemDataComponent>& itemDatas,
                   const ComponentStore<ZTransformComponent>& zTransforms,
                   sf::Color colorLOOT) {
        for (Entity loot : itemDatas.entityList()) {
            if (!transforms.has(loot)) continue;
            const auto& transform = transforms.get(loot);

            float z = zTransforms.has(loot) ? zTransforms.get(loot).z : 0.0f;
            float centerY = transform.position.y - z;

            sf::CircleShape lootCircle(8.0f);
            lootCircle.setOrigin({8.0f, 8.0f});
            lootCircle.setPosition({transform.position.x, centerY});
            lootCircle.setFillColor(colorLOOT);
            lootCircle.setOutlineColor(sf::Color(0, 150, 0));
            lootCircle.setOutlineThickness(2.0f);
            window.draw(lootCircle);
        }
    }

    void renderBombs(sf::RenderWindow& window, const ComponentStore<TransformComponent>& transforms,
                    const ComponentStore<BombComponent>& bombs,
                    const ComponentStore<ZTransformComponent>& zTransforms) {
        for (Entity entity : bombs.entityList()) {
            if (!transforms.has(entity)) continue;
            const auto& transform = transforms.get(entity);
            const auto& bomb = bombs.get(entity);

            float z = zTransforms.has(entity) ? zTransforms.get(entity).z : 0.0f;

            float centerX = transform.position.x;
            float centerY = transform.position.y - z;

            sf::CircleShape shadow(12.0f);
            shadow.setOrigin({12.0f, 6.0f});
            shadow.setScale({1.0f, 0.5f});
            shadow.setFillColor(sf::Color(0, 0, 0, 80));
            shadow.setPosition({centerX, transform.position.y});
            window.draw(shadow);

            sf::CircleShape bombCircle(15.0f);
            bombCircle.setOrigin({15.0f, 15.0f});
            bombCircle.setPosition({centerX, centerY});
            bombCircle.setFillColor(sf::Color(0, 0, 0));
            window.draw(bombCircle);

            float blinkAlpha = 150.0f + 100.0f * std::sin(bomb.fuseTimer * 10.0f);
            sf::CircleShape fuse(4.0f);
            fuse.setOrigin({4.0f, 4.0f});
            fuse.setPosition({centerX + 10.0f, centerY - 10.0f});
            fuse.setFillColor(sf::Color(255, 0, 0, static_cast<std::uint8_t>(blinkAlpha)));
            window.draw(fuse);
        }
    }

    void renderFence(sf::RenderWindow& window, GameWorld& world) {
        // 用围栏碰撞球计算围墙边界
        float minX = 0.0f, minY = 0.0f, maxX = 0.0f, maxY = 0.0f;
        bool first = true;
        for (Entity entity : world.fenceBalls) {
            if (!world.transforms.has(entity)) continue;
            const auto& trans = world.transforms.get(entity);
            float r = world.colliders.has(entity) ? world.colliders.get(entity).radius : 20.0f;
            float left = trans.position.x - r;
            float right = trans.position.x + r;
            float top = trans.position.y - r;
            float bottom = trans.position.y + r;
            if (first) {
                minX = left; minY = top; maxX = right; maxY = bottom;
                first = false;
            } else {
                if (left < minX) minX = left;
                if (top < minY) minY = top;
                if (right > maxX) maxX = right;
                if (bottom > maxY) maxY = bottom;
            }
        }
        if (first) return; // no fence balls

        const float wallThickness = 12.0f;
        sf::Color wallFill(60, 60, 80, 200);
        sf::Color wallStroke(100, 100, 130, 220);

        // 上围墙
        sf::RectangleShape topWall({maxX - minX, wallThickness});
        topWall.setPosition({minX, minY});
        topWall.setFillColor(wallFill);
        topWall.setOutlineColor(wallStroke);
        topWall.setOutlineThickness(2.0f);
        window.draw(topWall);

        // 下围墙
        sf::RectangleShape bottomWall({maxX - minX, wallThickness});
        bottomWall.setPosition({minX, maxY - wallThickness});
        bottomWall.setFillColor(wallFill);
        bottomWall.setOutlineColor(wallStroke);
        bottomWall.setOutlineThickness(2.0f);
        window.draw(bottomWall);

        // 左围墙
        sf::RectangleShape leftWall({wallThickness, maxY - minY});
        leftWall.setPosition({minX, minY});
        leftWall.setFillColor(wallFill);
        leftWall.setOutlineColor(wallStroke);
        leftWall.setOutlineThickness(2.0f);
        window.draw(leftWall);

        // 右围墙
        sf::RectangleShape rightWall({wallThickness, maxY - minY});
        rightWall.setPosition({maxX - wallThickness, minY});
        rightWall.setFillColor(wallFill);
        rightWall.setOutlineColor(wallStroke);
        rightWall.setOutlineThickness(2.0f);
        window.draw(rightWall);
    }

    void renderGrid(sf::RenderWindow& window) {
        const int gridSize = 100;
        const int gridMinX = 0;
        const int gridMinY = 0;
        const int gridMaxX = 2000;
        const int gridMaxY = 1200;

        // 懒初始化：第一次渲染时创建缓存
        if (!gridInitialized) {
            int lineCount = (gridMaxX - gridMinX) / gridSize + 1;
            int rowCount = (gridMaxY - gridMinY) / gridSize + 1;
            int totalVertices = (lineCount + rowCount) * 2;
            cachedGridLines = sf::VertexArray(sf::PrimitiveType::Lines, totalVertices);

            int idx = 0;
            for (int x = gridMinX; x <= gridMaxX; x += gridSize) {
                cachedGridLines[idx].position = sf::Vector2f(x, gridMinY);
                cachedGridLines[idx].color = sf::Color(100, 100, 120, 100);
                cachedGridLines[idx + 1].position = sf::Vector2f(x, gridMaxY);
                cachedGridLines[idx + 1].color = sf::Color(100, 100, 120, 100);
                idx += 2;
            }
            for (int y = gridMinY; y <= gridMaxY; y += gridSize) {
                cachedGridLines[idx].position = sf::Vector2f(gridMinX, y);
                cachedGridLines[idx].color = sf::Color(100, 100, 120, 100);
                cachedGridLines[idx + 1].position = sf::Vector2f(gridMaxX, y);
                cachedGridLines[idx + 1].color = sf::Color(100, 100, 120, 100);
                idx += 2;
            }
            gridInitialized = true;
        }

        window.draw(cachedGridLines);
    }

};

#else

#include "core/GameWorld.h"

class RenderSystem {
public:
    template<typename... Args>
    void update(Args&&...) const {}
};

#endif  // ENABLE_SFML

// Systems/CameraSystem.hpp
#pragma once
#include <SFML/Graphics/RenderWindow.hpp>
#include "ComponentManager.hpp"
#include "Components.hpp"
#include "Entity.hpp"
#include <cmath>

class CameraSystem {
public:
    CameraSystem(sf::RenderWindow& window, ComponentManager& cm, Entity player)
        : window(window), cm(cm), player(player) {
        baseView = window.getView();
    }

    void update(float dt) {
        auto* cam = cm.getComponent<CameraComponent>(player);
        auto* pos = cm.getComponent<PositionComponent>(player);
        auto* vel = cm.getComponent<VelocityComponent>(player);
        if (!pos || !vel || !cam) return;

        //1. 基础中心跟随玩家
        sf::Vector2f center = pos->position;

        // 2. 计算鼠标在世界坐标
        sf::Vector2i mousePixel = sf::Mouse::getPosition(window);
        sf::Vector2f mouseWorld = window.mapPixelToCoords(mousePixel);

        // 3. 方向和距离
        sf::Vector2f delta = mouseWorld - pos->position;
        float dist = std::sqrt(delta.x * delta.x + delta.y * delta.y);
        if (dist > 0.01f) {
            // 单位向量
            sf::Vector2f dir = delta / dist;
            // 按距离比例计算偏移（不超过 maxOffset）
            float factor = std::min(dist / 500.f, 1.f);
            sf::Vector2f offset = {
                dir.x * cam->maxOffset.x * factor,
                dir.y * cam->maxOffset.y * factor
            };
            center += offset;
        }


        // 计算归一化速度因子
        float vx = vel->velocity.x;
        float vy = vel->velocity.y;
        float normX = std::min(std::abs(vx) / maxSpeed, 1.f);
        float normY = std::min(std::abs(vy) / maxSpeed, 1.f);

        shakeTime += dt;

        // 改为相同方向的抖动
        float offsetX = std::sin(shakeTime * frequency) * amplitude * normX;
        float offsetY = std::sin(shakeTime * frequency) * amplitude * normY;

        center.x += offsetX;
        center.y += offsetY;

        sf::View view = baseView;
        view.setCenter(center);
        window.setView(view);
    }

private:
    sf::RenderWindow& window;
    ComponentManager& cm;
    Entity player;
    sf::View baseView;

    float shakeTime = 0.f;

    // 水平摇曳参数
    float frequency = 10.f;
    float amplitude = 10.f;
    float maxSpeed = 300.f;

    // 垂直摇曳参数
    //float frequency = 6.f;     // 可调成不同频率
    //float amplitudeY = 10.f;    // Y方向通常较轻微
    //float maxSpeedY = 300.f;
};


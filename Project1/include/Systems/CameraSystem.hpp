// Systems/CameraSystem.hpp
#pragma once
#include <SFML/Graphics/RenderWindow.hpp>
#include "ComponentManager.hpp"
#include "Components.hpp"
#include "Entity.hpp"
#include <cmath>

//class CameraSystem {
//public:
//    CameraSystem(sf::RenderWindow& window, ComponentManager& cm, Entity player)
//        : window(window), cm(cm), player(player) {}
//
//    void update() {
//        auto* cam = cm.getComponent<CameraComponent>(player);
//        auto* pos = cm.getComponent<PositionComponent>(player);
//        if (!cam || !pos) return;
//
//        // 1. 基础中心跟随玩家
//        sf::Vector2f center = pos->position;
//
//        // 2. 计算鼠标在世界坐标
//        sf::Vector2i mousePixel = sf::Mouse::getPosition(window);
//        sf::Vector2f mouseWorld = window.mapPixelToCoords(mousePixel);
//
//        // 3. 方向和距离
//        sf::Vector2f delta = mouseWorld - pos->position;
//        float dist = std::sqrt(delta.x * delta.x + delta.y * delta.y);
//        if (dist > 0.01f) {
//            // 单位向量
//            sf::Vector2f dir = delta / dist;
//            // 按距离比例计算偏移（不超过 maxOffset）
//            float factor = std::min(dist / 500.f, 1.f);
//            sf::Vector2f offset = { dir.x * cam->maxOffset.x * factor,
//                                    dir.y * cam->maxOffset.y * factor };
//            center += offset;
//        }
//
//        // 4. 应用到视图
//        sf::View view = window.getView();
//        view.setCenter(center);
//        window.setView(view);
//    }
//
//private:
//    sf::RenderWindow& window;
//    ComponentManager& cm;
//    Entity player;
//};

class CameraSystem {
public:
    CameraSystem(sf::RenderWindow& window, ComponentManager& cm, Entity player)
        : window(window), cm(cm), player(player) {
        baseView = window.getView();
    }

    void update(float dt) {
        auto* pos = cm.getComponent<PositionComponent>(player);
        auto* vel = cm.getComponent<VelocityComponent>(player);
        if (!pos || !vel) return;

        // 1. 基础中心
        sf::Vector2f center = pos->position;

        // 2. 根据水平速度添加摇曳
        float vx = vel->velocity.x;
        float norm = std::min(std::abs(vx) / maxSpeed, 1.f);
        // 正弦抖动
        shakeTime += dt;
        float offsetX = std::sin(shakeTime * frequency) * amplitude * norm;
        center.x += offsetX;

        // 3. 应用视图
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
    float frequency = 10.f; // 抖动频率
    float amplitude = 20.f; // 最大抖动幅度
    float maxSpeed = 400.f; // 用于归一化速度
};

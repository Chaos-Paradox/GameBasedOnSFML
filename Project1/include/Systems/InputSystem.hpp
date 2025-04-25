#pragma once
#include <SFML/Window/Keyboard.hpp>
#include "ComponentManager.hpp"
#include "Components.hpp"
#include <cmath>

class InputSystem {
public:
    InputSystem(ComponentManager& cm, Entity player)
        : cm(cm), player(player) {}

    void update() {
        auto* vel = cm.getComponent<VelocityComponent>(player);
        if (!vel) return;

        // 初始化速度
        vel->velocity = { 0, 0 };

        // 水平与垂直按键
        float vx = 0.f, vy = 0.f;

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)) vy -= 200.f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)) vy += 200.f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) vx -= 200.f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) vx += 200.f;

        // 归一化速度
        if (vx != 0.f || vy != 0.f) {
            float speed = std::sqrt(vx * vx + vy * vy);  // 计算总速度
            float maxSpeed = 200.f; // 最大速度限制

            // 如果总速度大于最大速度，进行归一化
            if (speed > maxSpeed) {
                float scale = maxSpeed / speed;
                vx *= scale;
                vy *= scale;
            }
        }

        // 最终速度赋值
        vel->velocity = { vx, vy };
    }

private:
    ComponentManager& cm;
    Entity player;
};

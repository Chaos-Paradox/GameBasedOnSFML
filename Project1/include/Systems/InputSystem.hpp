#pragma once
#include <SFML/Window/Keyboard.hpp>
#include "ComponentManager.hpp"
#include "Components.hpp"

class InputSystem {
public:
    InputSystem(ComponentManager& cm, Entity player)
        : cm(cm), player(player) {}
    
    void update() {
        auto* vel = cm.getComponent<VelocityComponent>(player);
        if (!vel) return;
        
        vel->velocity = {0, 0};
        
        // 跨平台输入：使用 Scancode (物理扫描码) 替代 Key (虚拟键码)
        // Scancode 在 Windows/macOS/Linux 上行为一致
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::W)) vel->velocity.y -= 200;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::S)) vel->velocity.y += 200;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::A)) vel->velocity.x -= 200;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::D)) vel->velocity.x += 200;
    }
    
private: 
    ComponentManager& cm; 
    Entity player;
};

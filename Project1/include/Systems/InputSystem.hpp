//#pragma once
//#include <SFML/Window/Keyboard.hpp>
//#include "EventBus.hpp"

//struct MoveEvent {
//    Entity entity;
//    sf::Vector2f dir;
//};
//
//class InputSystem {
//public:
//    InputSystem(EventBus& bus, int player) : bus(bus), player(player) {}
//    void update() {
//        sf::Vector2f d{ 0,0 };
//        //sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left)
//        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)) d.y -= 1;
//        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)) d.y += 1;
//        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) d.x -= 1;
//        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) d.x += 1;
//        /*if (d.x || d.y)*/ 
//        bus.emit(MoveEvent{ player.id,d });
//    }
//private:
//    EventBus& bus; Entity player;
//};

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
        vel->velocity = { 0,0 };
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)) vel->velocity.y -= 200;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)) vel->velocity.y += 200;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) vel->velocity.x -= 200;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) vel->velocity.x += 200;
    }
private: ComponentManager& cm; Entity player;
};

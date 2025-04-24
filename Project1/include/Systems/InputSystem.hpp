#pragma once
#include <SFML/Window/Keyboard.hpp>
#include "EventBus.hpp"

struct MoveEvent {
    Entity entity;
    sf::Vector2f dir;
};

class InputSystem {
public:
    InputSystem(EventBus& bus, int player) : bus(bus), player(player) {}
    void update() {
        sf::Vector2f d{ 0,0 };
        //sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left)
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)) d.y -= 1;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)) d.y += 1;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) d.x -= 1;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) d.x += 1;
        /*if (d.x || d.y)*/ 
        bus.emit(MoveEvent{ player.id,d });
    }
private:
    EventBus& bus; Entity player;
};
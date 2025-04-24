#include <SFML/Graphics.hpp>
#include "EventBus.hpp"
#include "ComponentManager.hpp"
#include "EntityManager.hpp"
#include "Components.hpp"
#include "Systems/InputSystem.hpp"
#include "Systems/MovementSystem.hpp"
#include "Systems/RenderSystem.hpp"
#include <iostream>

int main() {
    sf::RenderWindow window(sf::VideoMode({ 800, 600 }), "ECS Demo");
    EventBus bus;
    ComponentManager cm;
    EntityManager em(cm);

    // 创建玩家实体
    Entity player = em.create();
    cm.addComponent<PositionComponent>(player, { PositionComponent(400,300) });
    cm.addComponent<VelocityComponent>(player, { VelocityComponent(0,0) });
    sf::Texture tex;
    if (!tex.loadFromFile(".\\material\\pictures\\guy.png")) {
        // 错误处理：文件不存在或格式不支持
        std::cerr << "Failed to load texture\n";
    }
    //sf::Sprite sprite(tex);
    //sprite.setTexture(tex);
    //SpriteComponent spritecomponent(sprite);
    //auto sc = SpriteComponent(tex);
    cm.addComponent<SpriteComponent>(player, SpriteComponent(tex));

    // 系统
    InputSystem input(bus, player.id);
    MovementSystem move(bus, cm);
    RenderSystem render(window, cm);

    sf::Clock clock;
    while (window.isOpen()) {
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }
        }
        input.update();
        float dt = clock.restart().asSeconds();
        move.update(dt);
        render.update();
    }
}
